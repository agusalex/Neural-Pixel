<div align="center">

# Neural-Pixel
**A simple GUI wrapper for stable-diffusion.cpp written using C and GTK 4.**

[![ghcr](https://img.shields.io/badge/ghcr.io-agusalex%2Fneural--pixel-2f5dff?logo=github)](https://github.com/users/agusalex/packages/container/package/neural-pixel)
![Screenshot1](https://github.com/Luiz-Alcantara/Neural-Pixel/blob/main/screenshots/img1.png?raw=true)
</div>

With Neural Pixel, you can use Stable Diffusion on practically any GPU that supports Vulkan and has at least 3GB of VRAM. This is a simple way to generate your images without having to deal with CUDA/ROCm installations or hundreds of Python dependencies.

## Linux Requirements

- Linux Distro with kernel >= 5.14 (Tested on RHEL 9, Fedora 42, and Arch Linux).

- GTK 4, Vulkan(If using GPU) and libpng installed.
- Install deps on Arch (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo pacman -S gtk4 libpng zlib vulkan-icd-loader vulkan-radeon vulkan-tools
```

- Install deps on Debian/Ubuntu/Mint (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo apt install libgtk-4-1 libpng16-16 zlib1g vulkan-tools mesa-vulkan-drivers
```

- Install deps on Fedora/RHEL (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo dnf install gtk4 libpng zlib vulkan-tools mesa-vulkan-drivers
```

- Install deps on OpenSUSE (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo zypper install libgtk-4-1 libpng16-16 libz1 vulkan-tools libvulkan_radeon libvulkan1
```

- A GPU or iGPU with at least 3GB of VRAM for Vulkan.

## Running on Linux

Download the Linux bundle in the releases tab, extract it and run the "neural_pixel" binary.
If you want to see errors and details, start the application from a terminal and enable the "Terminal Verbose" option in "Extra Opts."

## Windows Requirements

- Microsoft Visual C++ Redistributable latest: [vc_redist](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).
- A GPU or iGPU with at least 3GB of VRAM for Vulkan.

## Running on Windows

Download the Windows bundle in the releases tab, extract it and run the "neural_pixel.bat" file.
You can directly run the "neural pixel" binary, but the variable that defines the dark theme will not be applied, so the app may look weird.

## Recommended checkpoints

- For realistic style: [Photon](https://huggingface.co/sam749/Photon-v1/blob/main/photon_v1.safetensors).
- For anime style: [SoteMix](https://civitai.com/models/72182/sotemix).

## Build

You'll need GTK 4 and libpng development libs, then just git clone this repository and run:
```
mkdir build && cd build && cmake .. && make
```
To build on Windows Use MSYS2.

To build sd.cpp follow the instructions on its github page: [Stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp).

## Web server (Docker) – A1111-style API powered by Vulkan sd.cpp

This repo now includes a minimal web server exposing a subset of AUTOMATIC1111’s API endpoints and a simple UI, but running the Vulkan `sd.cpp` backend. It serves endpoints like `/sdapi/v1/txt2img`, `/sdapi/v1/img2img`, `/sdapi/v1/samplers`, `/sdapi/v1/sd-models`, etc.

- Endpoints live at port `7860` by default.
- Models are read from `./models` (same layout used by the GUI):
  - `./models/checkpoints` (required)
  - `./models/vae`, `./models/loras`, `./models/embeddings`, etc.
- Outputs go to `./outputs`.
- The container expects `sd` (Vulkan) and/or `sd-cpu` binaries to be mounted at `/app/sd` and `/app/sd-cpu` respectively.

### Layout expected inside `./models`

```
models/
  checkpoints/
  vae/
  loras/
  embeddings/
  text_encoders/
  controlnet/
  upscale_models/
```

### Run with Docker Compose

1) Place your compiled `sd` (Vulkan) binary at `./sd` (and optionally `sd-cpu` at `./sd-cpu`).

2) Put your models under `./models/checkpoints`, `./models/vae`, etc.

3) Start using the published image:

```
docker compose up -d
```

Open `http://localhost:7860` for the minimal UI. Programmatic access via A1111-like API, e.g.:

```
POST http://localhost:7860/sdapi/v1/txt2img
{
  "prompt": "a photo of a cat",
  "width": 512,
  "height": 512,
  "steps": 30,
  "sampler_name": "dpm++2m",
  "scheduler": "karras",
  "model": "your_model.safetensors"  // file under models/checkpoints
}
```

Notes:
- NVIDIA GPUs inside Docker: ensure `--gpus all` or compose `device_requests` is configured, and host drivers are installed. For AMD/Intel (Mesa/Vulkan), pass-through `/dev/dri` and Vulkan ICDs as needed.
- The current server implements a subset of the A1111 API focused on txt2img/img2img. Extend `web/app/main.py` to add more endpoints as required.

### Direct Docker run (without compose)

- Pull the image (example tag):

```
docker pull ghcr.io/agusalex/neural-pixel:sha-d0ec575
```

- Run with NVIDIA GPU (Vulkan backend binary at `./sd`):

```
docker run -d --name neural-pixel \
  --gpus all \
  -p 7860:7860 \
  -v "$PWD/models:/app/models:ro" \
  -v "$PWD/outputs:/app/outputs" \
  -v "$PWD/sd:/app/sd:ro" \
  ghcr.io/agusalex/neural-pixel:latest
```

- CPU fallback (provide `./sd-cpu`):

```
docker run -d --name neural-pixel-cpu \
  -p 7860:7860 \
  -v "$PWD/models:/app/models:ro" \
  -v "$PWD/outputs:/app/outputs" \
  -v "$PWD/sd-cpu:/app/sd-cpu:ro" \
  ghcr.io/agusalex/neural-pixel:latest
```

## Notes

- SDLX Needs the VAE: [SDXL VAE FP16 Fix](https://huggingface.co/madebyollin/sdxl-vae-fp16-fix/blob/main/sdxl_vae.safetensors).

- At the moment the only upscaling model that works is: [RealESRGAN_x4plus_anime_6B.pth](https://github.com/xinntao/Real-ESRGAN/releases/download/v0.2.2.4/RealESRGAN_x4plus_anime_6B.pth).

## Credits

- This project is a GUI to [stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp) by [@leejet](https://github.com/leejet).

## Donations

- My PayPal: [link](https://www.paypal.com/donate/?hosted_button_id=G29L2QHNWDJHJ).

- Bitcoin
```
bc1qhxxgy52s2ps9j2gyzfxtykccrrpkzpu9uvnhhe
```
- Ethereum
```
0x5da0a849D04085C7F1943871F469cb77394aa84b
```
- Litecoin
```
ltc1q8fu7j3zyckl0w4e6m2q85xc69ywvtpnjzdjhvq
```
