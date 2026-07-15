/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**  Copyright 2026 Stephan Vedder
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ObjectPreview.cpp : W3D thumbnail of a thing template (Qt6 port).
//
// The MFC original borrowed the single WW3D device: it switched the render
// target to an offscreen 128x128 texture, rendered the model with a fresh
// camera, copied the render-target surface into a lockable system-memory
// surface (render targets aren't directly lockable), read the pixels and
// blitted them into the window.  This port does exactly the same and hands the
// pixels to a QImage.

// Qt headers first: the engine's GameMemory.h #defines newInstance, which would
// otherwise clobber QMetaObject::newInstance.
#include <QImage>
#include <QPainter>
#include <QPaintEvent>

#include "ObjectPreview.h"

#include "WorldBuilderDoc.h"
#include "wbview3d.h"
#include "Common/ThingTemplate.h"

#include "assetmgr.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "lightenvironment.h"
#include "rinfo.h"
#include "surfaceclass.h"
#include "texture.h"
#include "ww3d.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"

#define PREVIEW_WIDTH 128
#define PREVIEW_HEIGHT 128

ObjectPreview::ObjectPreview(QWidget *parent) :
	QWidget(parent),
	m_tTempl(NULL)
{
	setMinimumSize(64, 64);
}

ObjectPreview::~ObjectPreview()
{
}

QSize ObjectPreview::sizeHint(void) const
{
	return QSize(PREVIEW_WIDTH, PREVIEW_HEIGHT);
}

void ObjectPreview::setThingTemplate(const ThingTemplate *tTempl)
{
	m_tTempl = tTempl;
	m_image = renderTemplate(tTempl);
	update();
}

// ----------------------------------------------------------------------------
// Read a (non-lockable) render-target surface back into a QImage by copying it
// into a system-memory image surface first, then locking that.
static QImage readSurfaceToImage(IDirect3DSurface8 *surface)
{
	IDirect3DDevice8 *dev = DX8Wrapper::_Get_D3D_Device8();
	if (dev == NULL || surface == NULL)
		return QImage();

	D3DSURFACE_DESC desc;
	if (FAILED(surface->GetDesc(&desc)))
		return QImage();

	IDirect3DSurface8 *sysSurface = NULL;
	if (FAILED(dev->CreateImageSurface(desc.Width, desc.Height, desc.Format, &sysSurface)) || !sysSurface)
		return QImage();

	QImage image;
	if (SUCCEEDED(dev->CopyRects(surface, NULL, 0, sysSurface, NULL)))
	{
		D3DLOCKED_RECT lrect;
		if (SUCCEEDED(sysSurface->LockRect(&lrect, NULL, D3DLOCK_READONLY)))
		{
			// A8R8G8B8 / X8R8G8B8 in memory is B,G,R,A - the same byte order as
			// QImage::Format_RGB32, so rows copy straight across.
			image = QImage(desc.Width, desc.Height, QImage::Format_RGB32);
			const uchar *src = static_cast<const uchar*>(lrect.pBits);
			for (unsigned int y = 0; y < desc.Height; y++) {
				memcpy(image.scanLine(y), src + y * lrect.Pitch, desc.Width * 4);
			}
			sysSurface->UnlockRect();
		}
	}

	sysSurface->Release();
	return image;
}

// ----------------------------------------------------------------------------
QImage ObjectPreview::renderTemplate(const ThingTemplate *tt)
{
	if (tt == NULL)
		return QImage();

	// The shared WW3D device only exists once the 3d view has initialised it.
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	WbView3d *p3View = pDoc ? pDoc->GetActive3DView() : NULL;
	if (p3View == NULL || DX8Wrapper::_Get_D3D_Device8() == NULL)
		return QImage();

	ModelConditionFlags state;
	state.clear();
	AsciiString modelName = p3View->getBestModelName(tt, state);
	if (modelName.isEmpty() || strncmp(modelName.str(), "No ", 3) == 0)
		return QImage();

	WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();
	RenderObjClass *model = pMgr ? pMgr->Create_Render_Obj(modelName.str()) : NULL;
	if (model == NULL)
		return QImage();

	QImage result;

	TextureClass *objectTexture = DX8Wrapper::Create_Render_Target(PREVIEW_WIDTH, PREVIEW_HEIGHT);
	if (objectTexture)
	{
		const SphereClass sphere = model->Get_Bounding_Sphere();
		Real dist = sphere.Radius * 0.5f;
		model->Set_Position(Vector3(-sphere.Center.X, -sphere.Center.Y, -sphere.Center.Z));

		DX8Wrapper::Set_Render_Target_With_Z(objectTexture);

		CameraClass *camera = NEW_REF(CameraClass, ());
		Matrix3D camTran;
		camTran.Look_At(Vector3(dist * 2, dist * 2, dist), Vector3(0.0f, 0.0f, 0.0f), 0);
		camera->Set_Transform(camTran);
		camera->Set_View_Plane(Vector2(-1, -1), Vector2(+1, +1));
		camera->Set_Clip_Planes(0.995f, 600.0f);

		WW3D::Begin_Render(true, true, Vector3(0.5f, 0.5f, 0.5f));
		RenderInfoClass rinfo(*camera);
		LightEnvironmentClass lightEnv;
		rinfo.light_environment = &lightEnv;
		lightEnv.Reset(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f));
		WW3D::Render(*model, rinfo);
		WW3D::End_Render(false);

		// Restore the main back buffer as the render target.
		DX8Wrapper::Set_Render_Target((IDirect3DSurface8 *)NULL);

		SurfaceClass *surface = objectTexture->Get_Surface_Level();
		if (surface) {
			result = readSurfaceToImage(surface->Peek_D3D_Surface());
			REF_PTR_RELEASE(surface);
		}

		REF_PTR_RELEASE(camera);
		REF_PTR_RELEASE(objectTexture);
	}

	model->Release_Ref();
	return result;
}

// ----------------------------------------------------------------------------
void ObjectPreview::paintEvent(QPaintEvent * /*event*/)
{
	QPainter painter(this);
	QRect r = rect();
	if (!m_image.isNull()) {
		painter.drawImage(r, m_image);
	} else {
		painter.fillRect(r, QColor(128, 128, 128));
	}
	painter.setPen(QColor(0, 0, 0));
	painter.drawRect(r.adjusted(0, 0, -1, -1));
}
