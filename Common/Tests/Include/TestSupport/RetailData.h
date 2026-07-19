/*
**	Command & Conquer Generals(tm)
**	Command & Conquer Generals Zero Hour(tm)
**
**	Locating the retail game data used by tests.
**
**	The retail .big files are not redistributable, so they cannot live in the
**	repository. CI downloads them from a private S3 bucket and points the
**	environment variables below at the extracted archives. Builds without a
**	game install (most local checkouts, and pull requests from forks, which do
**	not receive repository secrets) simply skip the tests that need them.
*/

#pragma once

#include <gtest/gtest.h>

#include <cstdlib>

#define SAGE_RETAIL_DATA_ENV_GENERALS "SAGE_RETAIL_DATA_DIR_GENERALS"
#define SAGE_RETAIL_DATA_ENV_GENERALSMD "SAGE_RETAIL_DATA_DIR_GENERALSMD"

// Returns the configured retail data directory, or nullptr when unset/empty.
inline const char* getRetailDataDir(const char* envVar)
{
	const char* dir = std::getenv(envVar);
	return (dir != nullptr && dir[0] != '\0') ? dir : nullptr;
}

// Declares `varName` as the retail data directory, or skips the test if the
// environment variable is not set. `envVar` may be any expression yielding a
// name, so parameterised tests can pass one in per case.
#define SAGE_REQUIRE_RETAIL_DATA(varName, envVar)                                          \
	const char* varName##_env = (envVar);                                                  \
	const char* varName = getRetailDataDir(varName##_env);                                 \
	if (varName == nullptr)                                                                \
	GTEST_SKIP() << "Set " << varName##_env << " to a retail game data directory to run this test."
