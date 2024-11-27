#include <winrt/Windows.Foundation.h>

#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>

#include <winrt/base.h>
#include <d3d11_4.h>
#include <dxgi1_2.h>

#include <winrt/windows.graphics.directx.direct3d11.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <winrt/impl/windows.graphics.capture.2.h>

inline auto CreateD3D11Device(D3D_DRIVER_TYPE const type, UINT flags, winrt::com_ptr<ID3D11Device>& device) {
    WINRT_ASSERT(!device);

    return D3D11CreateDevice(nullptr, type, nullptr, flags, nullptr, 0, D3D11_SDK_VERSION, device.put(),
        nullptr, nullptr);
}

inline auto CreateD3D11Device(UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT) {
    winrt::com_ptr<ID3D11Device> device;
    HRESULT hr = CreateD3D11Device(D3D_DRIVER_TYPE_HARDWARE, flags, device);
    if(DXGI_ERROR_UNSUPPORTED == hr) {
        hr = CreateD3D11Device(D3D_DRIVER_TYPE_WARP, flags, device);
    }

    winrt::check_hresult(hr);
    return device;
}

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device) {
    winrt::com_ptr<::IInspectable> d3d_device;
    winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
    return d3d_device.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

class TestReceiver {
public:
    void OnFrameArrived(winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender, winrt::Windows::Foundation::IInspectable const&) {
        printf("Frame arrived!\n");
    }
};

extern "C" {
    void capture() {
        auto d3dDevice = CreateD3D11Device();
        auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device = CreateDirect3DDevice(dxgiDevice.get());

        HMONITOR monitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        auto interop_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem, IGraphicsCaptureItemInterop>();
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = {nullptr};
        winrt::check_hresult(interop_factory->CreateForMonitor(monitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item)));

        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(device, winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, item.Size());
        winrt::Windows::Graphics::Capture::GraphicsCaptureSession session = pool.CreateCaptureSession(item);

        TestReceiver receiver;
        pool.FrameArrived({&receiver, &TestReceiver::OnFrameArrived});
        session.StartCapture();
        printf("Capture started!\n");
        Sleep(5000);
    }
}
