#include "customrtstage.hpp"
#include <core/foray_shadermanager.hpp>

void CustomRtStage::Init(foray::core::Context *context, foray::scene::Scene *scene, foray::core::ManagedImage *envmap, foray::core::ManagedImage *noiseSource)
{
    mContext = context;
    mScene = scene;
    if (!!envmap)
    {
        mEnvMap.Create(context, envmap);
    }
    if (!!noiseSource)
    {
        mNoiseSource.Create(context, noiseSource, false);
        VkSamplerCreateInfo samplerCi{.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                      .magFilter = VkFilter::VK_FILTER_NEAREST,
                                      .minFilter = VkFilter::VK_FILTER_NEAREST,
                                      .addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                      .addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                      .addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                      .anisotropyEnable = VK_FALSE,
                                      .compareEnable = VK_FALSE,
                                      .minLod = 0,
                                      .maxLod = 0,
                                      .unnormalizedCoordinates = VK_FALSE};
        foray::AssertVkResult(vkCreateSampler(context->Device(), &samplerCi, nullptr, &mNoiseSource.Sampler));
    }
    RaytracingStage::Init();
}
void CustomRtStage::CreateRaytraycingPipeline()
{
    mRaygen.Create(mContext);
    mDefault_AnyHit.Create(mContext);
    mDefault_ClosestHit.Create(mContext);
    mDefault_Miss.Create(mContext);

    mPipeline.GetRaygenSbt().SetGroup(0, &(mRaygen.Module));
    mPipeline.GetMissSbt().SetGroup(0, &(mDefault_Miss.Module));
    mPipeline.GetHitSbt().SetGroup(0, &(mDefault_ClosestHit.Module), &(mDefault_AnyHit.Module), nullptr);
    RaytracingStage::CreateRaytraycingPipeline();
}

void CustomRtStage::OnShadersRecompiled()
{
    foray::core::ShaderManager& shaderCompiler = foray::core::ShaderManager::Instance();
    bool rebuild = shaderCompiler.HasShaderBeenRecompiled(mRaygen.Path) || shaderCompiler.HasShaderBeenRecompiled(mDefault_AnyHit.Path) || shaderCompiler.HasShaderBeenRecompiled(mDefault_ClosestHit.Path) || shaderCompiler.HasShaderBeenRecompiled(mDefault_Miss.Path);
    if (rebuild)
    {
        ReloadShaders();
    }
}
void CustomRtStage::Destroy()
{
    RaytracingStage::Destroy();
}

void CustomRtStage::DestroyShaders()
{
    mRaygen.Destroy();
    mDefault_AnyHit.Destroy();
    mDefault_ClosestHit.Destroy();
    mDefault_Miss.Destroy();
}

void CustomRtStage::RtStageShader::Create(foray::core::Context *context)
{
    Module.LoadFromSpirv(context, Path);
}
void CustomRtStage::RtStageShader::Destroy()
{
    Module.Destroy();
}
