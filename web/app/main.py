from __future__ import annotations

import base64
import os
import shutil
import subprocess
import time
from pathlib import Path
from typing import List, Optional

from fastapi import FastAPI, HTTPException, Request
from fastapi.responses import FileResponse, HTMLResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel, Field
from pydantic import ConfigDict


# ------------------------------
# Basic constants and defaults (mirroring constants.c)
# ------------------------------
MODELS_DIR = Path(os.getenv("MODELS_DIR", "./models")).resolve()
CHECKPOINTS_DIR = MODELS_DIR / "checkpoints"
CLIPS_DIR = MODELS_DIR / "clips"
CONTROLNET_DIR = MODELS_DIR / "controlnet"
EMBEDDINGS_DIR = MODELS_DIR / "embeddings"
LORAS_DIR = MODELS_DIR / "loras"
TEXT_ENCODERS_DIR = MODELS_DIR / "text_encoders"
UPSCALES_DIR = MODELS_DIR / "upscale_models"
VAES_DIR = MODELS_DIR / "vae"
OUTPUTS_DIR = Path(os.getenv("OUTPUTS_DIR", "./outputs")).resolve()


LIST_RESOLUTIONS = [
    64, 128, 192, 256, 320, 384, 448, 512, 576,
    640, 704, 768, 832, 896, 960, 1024, 1088,
    1152, 1216, 1280,
]
LIST_STEPS = [1, 2, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60]
LIST_SAMPLERS = [
    "euler", "euler_a", "heun", "dpm2",
    "dpm++2s_a", "dpm++2m", "dpm++2mv2",
    "ipndm", "ipndm_v", "lcm",
]
LIST_SCHEDULES = ["discrete", "karras", "exponential", "ays", "gits"]

DEFAULTS = {
    "sampler_name": "dpm++2m",
    "scheduler": "karras",
    "steps": 30,
    "width": 512,
    "height": 512,
    "batch_size": 1,
    "cfg_scale": 6.0,
    "denoise_strength": 0.75,
}


# ------------------------------
# API models (subset matching AUTOMATIC1111)
# ------------------------------
class Txt2ImgRequest(BaseModel):
    model_config = ConfigDict(extra="ignore")
    prompt: str = Field("")
    negative_prompt: str = Field("")
    steps: int = Field(DEFAULTS["steps"])
    sampler_name: str = Field(DEFAULTS["sampler_name"])
    scheduler: str = Field(DEFAULTS["scheduler"])
    width: int = Field(DEFAULTS["width"])
    height: int = Field(DEFAULTS["height"])
    batch_size: int = Field(DEFAULTS["batch_size"])
    n_iter: int = Field(1)
    cfg_scale: float = Field(DEFAULTS["cfg_scale"])
    seed: int = Field(-1)
    denoising_strength: Optional[float] = Field(None)  # used for img2img; ignored here
    restore_faces: Optional[bool] = Field(False)
    tiling: Optional[bool] = Field(False)
    # backend-specific extras
    model: Optional[str] = None  # filename under models/checkpoints
    vae: Optional[str] = None  # filename under models/vae
    cpu: Optional[bool] = False
    keep_clip_on_cpu: Optional[bool] = False
    keep_cnet_on_cpu: Optional[bool] = False
    keep_vae_on_cpu: Optional[bool] = False
    diffusion_fa: Optional[bool] = False


class ImageResponse(BaseModel):
    images: List[str]  # base64 PNG
    parameters: dict
    info: str


class Img2ImgRequest(BaseModel):
    model_config = ConfigDict(extra="ignore")
    prompt: str = Field("")
    negative_prompt: str = Field("")
    steps: int = Field(DEFAULTS["steps"])
    sampler_name: str = Field(DEFAULTS["sampler_name"])
    scheduler: str = Field(DEFAULTS["scheduler"])
    width: int = Field(DEFAULTS["width"])  # optional; sd.cpp reads from input image
    height: int = Field(DEFAULTS["height"])  # optional
    batch_size: int = Field(1)
    n_iter: int = Field(1)
    cfg_scale: float = Field(DEFAULTS["cfg_scale"])
    seed: int = Field(-1)
    denoising_strength: float = Field(0.75)
    init_images: List[str] = Field(default_factory=list)  # base64 PNG/JPEG
    # backend-specific
    model: Optional[str] = None
    vae: Optional[str] = None
    cpu: Optional[bool] = False
    keep_clip_on_cpu: Optional[bool] = False
    keep_cnet_on_cpu: Optional[bool] = False
    keep_vae_on_cpu: Optional[bool] = False
    diffusion_fa: Optional[bool] = False


# ------------------------------
# App setup
# ------------------------------
app = FastAPI(title="Neural-Pixel Web API (A1111-compatible subset)")

static_dir = Path(__file__).resolve().parent.parent / "static"
app.mount("/static", StaticFiles(directory=str(static_dir)), name="static")


def ensure_dirs() -> None:
    OUTPUTS_DIR.mkdir(parents=True, exist_ok=True)
    for d in [
        MODELS_DIR,
        CHECKPOINTS_DIR,
        CLIPS_DIR,
        CONTROLNET_DIR,
        EMBEDDINGS_DIR,
        LORAS_DIR,
        TEXT_ENCODERS_DIR,
        UPSCALES_DIR,
        VAES_DIR,
    ]:
        d.mkdir(parents=True, exist_ok=True)


@app.get("/", response_class=HTMLResponse)
def index(_: Request):
    index_html = static_dir / "index.html"
    if not index_html.exists():
        raise HTTPException(status_code=404, detail="UI not found")
    return HTMLResponse(index_html.read_text(encoding="utf-8"))


@app.get("/sdapi/v1/samplers")
def list_samplers():
    return [{"name": s, "aliases": [], "options": {}} for s in LIST_SAMPLERS]


@app.get("/sdapi/v1/schedulers")
def list_schedulers():
    return [{"name": s} for s in LIST_SCHEDULES]


@app.get("/sdapi/v1/sd-models")
def sd_models():
    ensure_dirs()
    models = []
    if CHECKPOINTS_DIR.exists():
        for p in sorted(CHECKPOINTS_DIR.iterdir()):
            if p.is_file():
                models.append({
                    "title": p.name,
                    "model_name": p.stem,
                    "filename": str(p),
                })
    return models


@app.get("/sdapi/v1/sd-vae")
def sd_vaes():
    ensure_dirs()
    vaes = []
    if VAES_DIR.exists():
        for p in sorted(VAES_DIR.iterdir()):
            if p.is_file():
                vaes.append({
                    "model_name": p.stem,
                    "filename": p.name,
                })
    return vaes


@app.get("/sdapi/v1/options")
def options_get():
    return {
        "sd_model_checkpoint": None,
        "sd_vae": None,
        "sd_checkpoint_cache": 0,
        "samples_save": True,
        "samples_format": "png",
        "show_progress_every_n_steps": 5,
    }


def which_sd_binary(use_cpu: bool) -> str:
    # Prefer absolute path under PATH, then local working dir
    def is_exec(p: Path) -> bool:
        try:
            return p.exists() and p.is_file() and os.access(p, os.X_OK)
        except Exception:
            return False

    name = "sd-cpu" if use_cpu else "sd"
    candidates = [
        name,
        f"/app/{name}",
        f"/app/bin/{name}",
        f"/usr/local/bin/{name}",
        f"/usr/bin/{name}",
        f"/bin/{name}",
        f"./{name}",
    ]
    for cand in candidates:
        if os.path.isabs(cand):
            p = Path(cand)
            if is_exec(p):
                return str(p)
        else:
            # search PATH
            found = shutil.which(cand)
            if found and is_exec(Path(found)):
                return found
    return name


def build_sd_command(req: Txt2ImgRequest, out_path: Path) -> List[str]:
    cmd: List[str] = [which_sd_binary(bool(req.cpu))]

    # model
    if req.model:
        cmd += ["-m", str(CHECKPOINTS_DIR / req.model)]

    # directories for embeddings/loras
    cmd += ["--lora-model-dir", str(LORAS_DIR), "--embd-dir", str(EMBEDDINGS_DIR)]

    # optional VAE
    if req.vae:
        cmd += ["--vae", str(VAES_DIR / req.vae)]
        if req.keep_vae_on_cpu:
            cmd += ["--vae-on-cpu"]

    # schedules and sampler
    if req.sampler_name in LIST_SAMPLERS:
        cmd += ["--sampling-method", req.sampler_name]
    if req.scheduler in LIST_SCHEDULES:
        cmd += ["--schedule", req.scheduler]

    # steps, batch
    if req.steps in LIST_STEPS:
        cmd += ["--steps", str(req.steps)]
    if req.batch_size > 1:
        cmd += ["-b", str(req.batch_size)]

    # cfg
    cmd += ["--cfg-scale", f"{float(req.cfg_scale):.1f}"]

    # width/height
    if req.width in LIST_RESOLUTIONS:
        cmd += ["-W", str(req.width)]
    if req.height in LIST_RESOLUTIONS:
        cmd += ["-H", str(req.height)]

    # seed
    cmd += ["-s", str(req.seed if req.seed is not None else -1)]

    # options
    if req.tiling:
        cmd += ["--vae-tiling"]
    if req.keep_clip_on_cpu:
        cmd += ["--clip-on-cpu"]
    if req.diffusion_fa:
        cmd += ["--diffusion-fa"]

    # prompts
    cmd += ["-p", req.prompt or "", "-n", req.negative_prompt or ""]

    # output
    cmd += ["-o", str(out_path)]
    return cmd


def build_sd_img2img_command(req: Img2ImgRequest, init_path: Path, out_path: Path) -> List[str]:
    cmd: List[str] = [which_sd_binary(bool(req.cpu))]
    cmd += ["-M", "img2img", "-i", str(init_path)]

    if req.model:
        cmd += ["-m", str(CHECKPOINTS_DIR / req.model)]

    cmd += ["--lora-model-dir", str(LORAS_DIR), "--embd-dir", str(EMBEDDINGS_DIR)]

    if req.vae:
        cmd += ["--vae", str(VAES_DIR / req.vae)]
        if req.keep_vae_on_cpu:
            cmd += ["--vae-on-cpu"]

    if req.sampler_name in LIST_SAMPLERS:
        cmd += ["--sampling-method", req.sampler_name]
    if req.scheduler in LIST_SCHEDULES:
        cmd += ["--schedule", req.scheduler]

    if req.steps in LIST_STEPS:
        cmd += ["--steps", str(req.steps)]
    if req.batch_size > 1:
        cmd += ["-b", str(req.batch_size)]

    cmd += ["--cfg-scale", f"{float(req.cfg_scale):.1f}"]
    cmd += ["--strength", f"{float(req.denoising_strength):.2f}"]
    cmd += ["-s", str(req.seed if req.seed is not None else -1)]

    if req.tiling:
        cmd += ["--vae-tiling"]
    if req.keep_clip_on_cpu:
        cmd += ["--clip-on-cpu"]
    if req.diffusion_fa:
        cmd += ["--diffusion-fa"]

    cmd += ["-p", req.prompt or "", "-n", req.negative_prompt or ""]
    cmd += ["-o", str(out_path)]
    return cmd


def read_file_b64(path: Path) -> str:
    with path.open("rb") as f:
        return base64.b64encode(f.read()).decode("utf-8")


@app.post("/sdapi/v1/txt2img", response_model=ImageResponse)
def txt2img(req: Txt2ImgRequest):
    ensure_dirs()

    if req.width not in LIST_RESOLUTIONS or req.height not in LIST_RESOLUTIONS:
        raise HTTPException(status_code=400, detail="width/height must be one of allowed resolutions")
    if req.sampler_name not in LIST_SAMPLERS:
        raise HTTPException(status_code=400, detail="invalid sampler_name")
    if req.scheduler not in LIST_SCHEDULES:
        raise HTTPException(status_code=400, detail="invalid scheduler")

    # Build output filename
    ts = time.strftime("%Y%m%d_%H%M%S")
    out_file = OUTPUTS_DIR / f"IMG_{ts}.png"

    cmd = build_sd_command(req, out_file)
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            text=True,
        )
    except FileNotFoundError:
        raise HTTPException(status_code=500, detail="sd binary not found in PATH or working dir")

    if proc.returncode != 0:
        raise HTTPException(status_code=500, detail=f"sd failed: {proc.stderr.strip() or proc.stdout.strip()}")

    if not out_file.exists():
        raise HTTPException(status_code=500, detail="output image not produced")

    img_b64 = read_file_b64(out_file)
    info = {
        "prompt": req.prompt,
        "negative_prompt": req.negative_prompt,
        "sampler_name": req.sampler_name,
        "scheduler": req.scheduler,
        "steps": req.steps,
        "cfg_scale": req.cfg_scale,
        "seed": req.seed,
        "width": req.width,
        "height": req.height,
        "batch_size": req.batch_size,
        "model": req.model,
        "vae": req.vae,
    }
    return ImageResponse(images=[img_b64], parameters=req.dict(), info=str(info))


@app.post("/sdapi/v1/img2img", response_model=ImageResponse)
def img2img(req: Img2ImgRequest):
    ensure_dirs()
    if not req.init_images:
        raise HTTPException(status_code=400, detail="init_images is required")

    # Decode first init image
    try:
        raw = base64.b64decode(req.init_images[0].split(",")[-1])
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"invalid base64: {e}")

    ts = time.strftime("%Y%m%d_%H%M%S")
    init_path = OUTPUTS_DIR / f"INIT_{ts}.png"
    with init_path.open("wb") as f:
        f.write(raw)

    out_file = OUTPUTS_DIR / f"IMG_{ts}.png"
    cmd = build_sd_img2img_command(req, init_path, out_file)
    try:
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
            text=True,
        )
    except FileNotFoundError:
        raise HTTPException(status_code=500, detail="sd binary not found in PATH or working dir")

    if proc.returncode != 0:
        raise HTTPException(status_code=500, detail=f"sd failed: {proc.stderr.strip() or proc.stdout.strip()}")
    if not out_file.exists():
        raise HTTPException(status_code=500, detail="output image not produced")

    img_b64 = read_file_b64(out_file)
    info = {
        "prompt": req.prompt,
        "negative_prompt": req.negative_prompt,
        "sampler_name": req.sampler_name,
        "scheduler": req.scheduler,
        "steps": req.steps,
        "cfg_scale": req.cfg_scale,
        "seed": req.seed,
        "denoising_strength": req.denoising_strength,
        "model": req.model,
        "vae": req.vae,
    }
    return ImageResponse(images=[img_b64], parameters=req.dict(), info=str(info))


@app.get("/outputs/{name}")
def get_output(name: str):
    p = OUTPUTS_DIR / name
    if not p.exists():
        raise HTTPException(status_code=404, detail="not found")
    return FileResponse(str(p))


# Health check
@app.get("/healthz")
def healthz():
    return {"status": "ok"}

