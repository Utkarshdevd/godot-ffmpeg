#!/usr/bin/env python3
"""
Generate a matrix of test video assets: Resolution x Codec x Bitrate.
Creates a 4K master first, then derives 720p and other variants.
Output: tests/assets/ (gitignored).
"""
import os
import subprocess
import sys

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "assets")
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Matrix: codec -> (ffmpeg encoder, extension, extra args for encoding)
CODECS_ALL = {
    "h264": ("libx264", "mp4", ["-pix_fmt", "yuv420p"]),
    "hevc": ("libx265", "mp4", ["-pix_fmt", "yuv420p", "-tag:v", "hvc1"]),
    "av1": ("libsvtav1", "mkv", ["-preset", "8", "-b:v", "1M"]), 
}

# Resolutions: (width, height)
RESOLUTIONS = {
    "720p": (1280, 720),
    "4k": (3840, 2160),
}

# Bitrate presets (name -> approximate bitrate for 4K; scaled for 720p)
BITRATES = {
    "low": {"4k": "4M", "720p": "1M"},
    "high": {"4k": "20M", "720p": "5M"},
}


def get_available_codecs():
    """Return only codecs whose encoder is available in this FFmpeg build."""
    try:
        out = subprocess.run(
            ["ffmpeg", "-encoders"],
            capture_output=True,
            text=True,
            check=True,
            timeout=10,
        )
        encoders_out = (out.stderr or out.stdout) or ""
    except (subprocess.CalledProcessError, FileNotFoundError, subprocess.TimeoutExpired):
        print("⚠️  Warning: Could not query ffmpeg encoders. Defaulting to H.264 only.")
        return {"h264": CODECS_ALL["h264"]}

    available = {}
    for codec_key, (enc, ext, extra) in CODECS_ALL.items():
        # Check if the specific encoder (e.g., libsvtav1) is in the output
        if enc in encoders_out:
            available[codec_key] = (enc, ext, extra)
        else:
            print(f"⚠️  Skipping {codec_key}: Encoder '{enc}' not found in ffmpeg.")
            
    if not available:
        print("❌ Error: No compatible encoders found.")
        sys.exit(1)
        
    return available


def run_ffmpeg(cmd, description):
    """Run ffmpeg; raise on failure."""
    try:
        # We use -hide_banner and -loglevel error to keep output clean
        full_cmd = ["ffmpeg", "-hide_banner", "-loglevel", "error"] + cmd[1:]
        subprocess.run(
            full_cmd,
            check=True,
            capture_output=True,
            text=True,
        )
    except subprocess.CalledProcessError as e:
        print(f"❌ FFmpeg failed ({description}): {e.stderr or e.stdout}", file=sys.stderr)
        raise
    except FileNotFoundError:
        print("❌ FFmpeg not found. Please install ffmpeg and ensure it is on PATH.", file=sys.stderr)
        sys.exit(1)


def generate_master_4k():
    """Generate a 4K master (synthetic pattern) as source for all derivatives."""
    master_path = os.path.join(OUTPUT_DIR, "master_4k.mp4")
    if os.path.isfile(master_path):
        print(f"✅ 4K master already exists: {master_path}")
        return master_path

    print("🎨 Generating 4K Master Pattern (this may take a moment)...")
    
    # FIX: Added -map 0:v and -map 1:a so it combines both inputs
    cmd = [
        "ffmpeg", "-y",
        "-f", "lavfi", "-i", "testsrc=duration=10:size=3840x2160:rate=30",
        "-f", "lavfi", "-i", "sine=frequency=440:duration=10:sample_rate=48000",
        "-map", "0:v", # Take Video from Input 0 (testsrc)
        "-map", "1:a", # Take Audio from Input 1 (sine)
        "-c:v", "libx264", "-preset", "medium", "-b:v", "20M",
        "-pix_fmt", "yuv420p",
        "-c:a", "aac", "-b:a", "128k",
        "-shortest", # Stop when the shortest stream ends
        master_path,
    ]
    run_ffmpeg(cmd, "4K master")
    print(f"✅ Generated 4K master: {master_path}")
    return master_path


def derive_video(master_path, codec_key, resolution_key, bitrate_key, codecs):
    """Derive a single asset from the master: transcode to codec and scale to resolution."""
    codec_info = codecs[codec_key]
    enc, ext, extra = codec_info[0], codec_info[1], codec_info[2]
    w, h = RESOLUTIONS[resolution_key]
    br = BITRATES[bitrate_key][resolution_key]
    
    out_name = f"test_{codec_key}_{resolution_key}_{bitrate_key}.{ext}"
    out_path = os.path.join(OUTPUT_DIR, out_name)
    
    if os.path.isfile(out_path):
        # Optional: Print less noise if skipping
        # print(f"Skip (exists): {out_name}")
        return

    print(f"🎬 Generating: {out_name}...")
    
    cmd = [
        "ffmpeg", "-y", "-i", master_path,
        "-vf", f"scale={w}:{h}",
        "-c:v", enc, "-b:v", br,
        "-c:a", "copy", # Copy audio from master (much faster)
    ] + extra + [out_path]
    
    run_ffmpeg(cmd, out_name)


def main():
    print(f"🚀 Starting Asset Generation in {OUTPUT_DIR}")
    
    # 1. Detect Codecs
    codecs = get_available_codecs()
    print(f"ℹ️  Active Codecs: {', '.join(codecs.keys())}")
    
    # 2. Generate Master
    master = generate_master_4k()
    
    # 3. Generate Derivatives
    for codec_key in codecs:
        for res_key in RESOLUTIONS:
            for br_key in BITRATES:
                derive_video(master, codec_key, res_key, br_key, codecs)
                
    print(f"\n✅ Done. Assets are located in: {OUTPUT_DIR}")


if __name__ == "__main__":
    main()