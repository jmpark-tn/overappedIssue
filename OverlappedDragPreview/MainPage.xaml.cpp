//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <robuffer.h>
#include <wrl/client.h>

using namespace OverlappedDragPreview;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;

struct BufferHelper {
    Microsoft::WRL::ComPtr<IBufferByteAccess> readBuffer;
    int width;
    int height;
};

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}

void OverlappedDragPreview::MainPage::Image_DragStarting(Windows::UI::Xaml::UIElement^ sender, Windows::UI::Xaml::DragStartingEventArgs^ args)
{
    args->Cancel = false;
    auto deferal = args->GetDeferral();

    auto bufferHelper = std::make_shared<BufferHelper>();
    auto rtb = ref new RenderTargetBitmap();
    auto task = concurrency::create_task(rtb->RenderAsync(sender))
        .then([rtb] {
        return rtb->GetPixelsAsync();
    }).then([rtb, bufferHelper](IBuffer^ buffer) {
        Microsoft::WRL::ComPtr<IBufferByteAccess> readBuffer;
        if (reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&readBuffer)) != S_OK) {
            throw ref new Platform::FailureException();
        }
        bufferHelper->width = rtb->PixelWidth;
        bufferHelper->height = rtb->PixelHeight;
        bufferHelper->readBuffer = readBuffer;
    }).then([bufferHelper, args, deferal] {
        unsigned int size = bufferHelper->width * bufferHelper->height * 4;
        auto destMemoryBuffer = ref new Windows::Storage::Streams::Buffer(size);
        destMemoryBuffer->Length = size;

        Microsoft::WRL::ComPtr<IBufferByteAccess> destBuffer;
        if (reinterpret_cast<IInspectable*>(destMemoryBuffer)->QueryInterface(IID_PPV_ARGS(&destBuffer)) != S_OK) {
            throw ref new Platform::FailureException();
        }
        uint32_t* destPixels = nullptr;
        destBuffer->Buffer(reinterpret_cast<uint8_t**>(&destPixels));

        uint32_t* sourcePixels = nullptr;
        bufferHelper->readBuffer->Buffer(reinterpret_cast<uint8_t**>(&sourcePixels));

        memcpy(destPixels, sourcePixels, size);

        auto bitmap = SoftwareBitmap::CreateCopyFromBuffer(destMemoryBuffer,
            Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
            bufferHelper->width, bufferHelper->height,
            Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied);;
        args->DragUI->SetContentFromSoftwareBitmap(bitmap);
        deferal->Complete();
    }).then([deferal](concurrency::task<void> lastTask) {
        try {
            lastTask.get();
        } catch (Platform::Exception^ exc) {
            deferal->Complete();
        }
    });
}
