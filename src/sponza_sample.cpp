#include "sponza_sample.hpp"
#include <gltfconvert/hsk_modelconverter.hpp>
#include <scenegraph/components/hsk_camera.hpp>
#include <scenegraph/components/hsk_freecameracontroller.hpp>

void ImportanceSamplingRtProject::Init()
{
    mWindow.DisplayMode(hsk::EDisplayMode::WindowedResizable);
    loadScene();
    ConfigureStages();
}

void ImportanceSamplingRtProject::Update(float delta)
{
    // mFreeFlightCameraController.Update(delta);
}

void ImportanceSamplingRtProject::OnEvent(hsk::Event::ptr event)
{
    auto buttonInput = std::dynamic_pointer_cast<hsk::EventInputBinary>(event);
    auto axisInput = std::dynamic_pointer_cast<hsk::EventInputAnalogue>(event);
    auto windowResized = std::dynamic_pointer_cast<hsk::EventWindowResized>(event);
    if (buttonInput)
    {
        spdlog::info("Device \"{}\" Button {} - {}", buttonInput->Device()->Name(), buttonInput->Button()->Name(), buttonInput->Pressed() ? "pressed" : "released");
    }
    if (axisInput)
    {
        spdlog::info("Device \"{}\" Axis {} - {}", axisInput->Device()->Name(), axisInput->Axis()->Name(), axisInput->State());
    }
    if (windowResized)
    {
        spdlog::info("Window resized w {} h {}", windowResized->Current().Width, windowResized->Current().Height);
    }
    mScene->InvokeOnEvent(event);
    // mFreeFlightCameraController.OnEvent(event);
}

void ImportanceSamplingRtProject::loadScene()
{
    // std::string fullFileName = hsk::MakeRelativePath("models/minimal.gltf");
    std::string fullFileName = hsk::MakeRelativePath("../glTF-Sample-Models/2.0/Avocado/glTF/Avocado.gltf");
    // std::string fullFileName = hsk::MakeRelativePath("../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");

    mScene = std::make_unique<hsk::NScene>(&mContext);
    {
        hsk::ModelConverter converter(mScene.get());
        converter.LoadGltfModel(fullFileName);
    }

    auto cameraNode = mScene->MakeNode();

    cameraNode->MakeComponent<hsk::NCamera>()->InitDefault();
    cameraNode->MakeComponent<hsk::FreeCameraController>();

    // propagate vk variables
    // mScene.Context(&mContext);
    // mScene.LoadFromFile(fullFileName);

    // if (mScene.GetCameras().size() == 0)
    // {
    //     throw hsk::Exception("Scene has no camera attached!");
    // }
    // mFreeFlightCameraController.Init(&mContext, mScene.GetCameras()[0].get(), &mOsManager);
}

void ImportanceSamplingRtProject::Cleanup()
{
    vkDeviceWaitIdle(mDevice);
    mScene->Cleanup();
    mGbufferStage.Destroy();
    DefaultAppBase::Cleanup();
}

void ImportanceSamplingRtProject::ConfigureStages()
{
    mGbufferStage.Init(&mContext, mScene.get());
    mSwapchainCopySourceImage = mGbufferStage.GetColorAttachmentByName(hsk::GBufferStage::Albedo);
}

void ImportanceSamplingRtProject::RecordCommandBuffer(hsk::FrameRenderInfo &renderInfo)
{
    mScene->Update(renderInfo);
    mGbufferStage.RecordFrame(renderInfo);
}

void ImportanceSamplingRtProject::OnResized(VkExtent2D size)
{
    mScene->InvokeOnResized(size);
    mGbufferStage.OnResized(size);
    mSwapchainCopySourceImage = mGbufferStage.GetColorAttachmentByName("Normal");
}
