#pragma once
#include <QWindow>

struct SDL_Window;
class QSdlWindow : public QWindow
{
public:
    QSdlWindow(QWindow *parent = nullptr) : QWindow(parent)
    {
    }

    ~QSdlWindow() override = default;

    void resizeEvent(QResizeEvent *event) override;
    void Initialize();

    SDL_Window *GetSDLWindow() const
    {
        return m_pSDLWindow;
    }

private:
    SDL_Window *m_pSDLWindow = NULL;
    void *m_pWindowId = NULL;
};