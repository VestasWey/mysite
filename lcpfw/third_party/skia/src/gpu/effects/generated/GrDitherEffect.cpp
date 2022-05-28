/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrDitherEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrDitherEffect.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLDitherEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLDitherEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrDitherEffect& _outer = args.fFp.cast<GrDitherEffect>();
        (void)_outer;
        auto range = _outer.range;
        (void)range;
        rangeVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag, kHalf_GrSLType,
                                                    "range");
        SkString _sample0 = this->invokeChild(0, args);
        fragBuilder->codeAppendf(
                R"SkSL(half4 color = %s;
half value;
@if (sk_Caps.integerSupport) {
    uint x = uint(sk_FragCoord.x);
    uint y = uint(sk_FragCoord.y) ^ x;
    uint m = (((((y & 1) << 5 | (x & 1) << 4) | (y & 2) << 2) | (x & 2) << 1) | (y & 4) >> 1) | (x & 4) >> 2;
    value = half(m) / 64.0 - 0.4921875;
} else {
    half4 bits = mod(half4(sk_FragCoord.yxyx), half4(2.0, 2.0, 4.0, 4.0));
    bits.zw = step(2.0, bits.zw);
    bits.xz = abs(bits.xz - bits.yw);
    value = dot(bits, half4(0.5, 0.25, 0.125, 0.0625)) - 0.46875;
}
return half4(clamp(color.xyz + value * %s, 0.0, color.w), color.w);
)SkSL",
                _sample0.c_str(), args.fUniformHandler->getUniformCStr(rangeVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrDitherEffect& _outer = _proc.cast<GrDitherEffect>();
        { pdman.set1f(rangeVar, (_outer.range)); }
    }
    UniformHandle rangeVar;
};
std::unique_ptr<GrGLSLFragmentProcessor> GrDitherEffect::onMakeProgramImpl() const {
    return std::make_unique<GrGLSLDitherEffect>();
}
void GrDitherEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                           GrProcessorKeyBuilder* b) const {}
bool GrDitherEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrDitherEffect& that = other.cast<GrDitherEffect>();
    (void)that;
    if (range != that.range) return false;
    return true;
}
GrDitherEffect::GrDitherEffect(const GrDitherEffect& src)
        : INHERITED(kGrDitherEffect_ClassID, src.optimizationFlags()), range(src.range) {
    this->cloneAndRegisterAllChildProcessors(src);
}
std::unique_ptr<GrFragmentProcessor> GrDitherEffect::clone() const {
    return std::make_unique<GrDitherEffect>(*this);
}
#if GR_TEST_UTILS
SkString GrDitherEffect::onDumpInfo() const { return SkStringPrintf("(range=%f)", range); }
#endif
GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrDitherEffect);
#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrDitherEffect::TestCreate(GrProcessorTestData* d) {
    float range = 1.0f - d->fRandom->nextRangeF(0.0f, 1.0f);
    return GrDitherEffect::Make(GrProcessorUnitTest::MakeChildFP(d), range);
}
#endif
