#ifndef _ES3FBOOLEANSTATEQUERYTESTS_HPP
#define _ES3FBOOLEANSTATEQUERYTESTS_HPP
/*-------------------------------------------------------------------------
 * drawElements Quality Program OpenGL ES 3.0 Module
 * -------------------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *//*!
 * \file
 * \brief Boolean State Query tests.
 *//*--------------------------------------------------------------------*/

#include "tcuDefs.hpp"
#include "tes3TestCase.hpp"

namespace deqp
{
namespace gles3
{
namespace Functional
{
namespace BooleanStateQueryVerifiers
{

class IsEnabledVerifier;
class GetBooleanVerifier;
class GetIntegerVerifier;
class GetInteger64Verifier;
class GetFloatVerifier;

} // BooleanStateQueryVerifiers

class BooleanStateQueryTests : public TestCaseGroup
{
public:
																BooleanStateQueryTests	(Context& context);
																~BooleanStateQueryTests	(void);

	void														init					(void);
	void														deinit					(void);

private:
																BooleanStateQueryTests	(const BooleanStateQueryTests& other);
	BooleanStateQueryTests&										operator=				(const BooleanStateQueryTests& other);

	BooleanStateQueryVerifiers::IsEnabledVerifier*				m_verifierIsEnabled;
	BooleanStateQueryVerifiers::GetBooleanVerifier*				m_verifierBoolean;
	BooleanStateQueryVerifiers::GetIntegerVerifier*				m_verifierInteger;
	BooleanStateQueryVerifiers::GetInteger64Verifier*			m_verifierInteger64;
	BooleanStateQueryVerifiers::GetFloatVerifier*				m_verifierFloat;
};

} // Functional
} // gles3
} // deqp

#endif // _ES3FBOOLEANSTATEQUERYTESTS_HPP
