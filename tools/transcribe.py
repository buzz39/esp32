#!/usr/bin/env python3
"""
NeoClone post-processing: WAV files -> timestamped transcript -> summary.

This is the "AI" half of the pendant. Runs on your PC, free, offline.

    pip install faster-whisper requests
    python transcribe.py D:\\REC_*.WAV --out notes\\

Options:
    --model   tiny | base | small | medium | large-v3   (default: small)
    --lang    auto | en | hi | ...                       (default: auto)
    --out     output directory for .md notes
    --ollama  Ollama base URL for summaries (default: http://localhost:11434)
              if Ollama isn't reachable, the transcript is still written.

Tip: `ollama pull llama3.2` then leave Ollama running to get summaries.
"""

from __future__ import annotations

import argparse
import datetime as dt
import glob
import os
import sys
from pathlib import Path

SUMMARY_PROMPT = """You are a note-taker. Read this raw transcript and produce:

1. **Summary** - 3-6 bullets, what actually happened / was decided.
2. **Action items** - every task, owner if stated, deadline if stated.
3. **Key facts** - names, numbers, dates, links worth remembering.

Be concise and concrete. Do not invent anything not in the transcript.
Transcript may contain ASR errors; use judgement.

TRANSCRIPT:
{transcript}
"""


def fmt_ts(seconds: float) -> str:
    m, s = divmod(int(seconds), 60)
    h, m = divmod(m, 60)
    return f"{h:02d}:{m:02d}:{s:02d}"


def transcribe(path: str, model, language: str | None):
    segments, info = model.transcribe(
        path,
        language=language,
        vad_filter=True,
        beam_size=1,
        condition_on_previous_text=False,
    )
    lines = []
    plain = []
    for seg in segments:
        text = seg.text.strip()
        if not text:
            continue
        lines.append(f"[{fmt_ts(seg.start)}] {text}")
        plain.append(text)
    return lines, " ".join(plain), info


def summarize(transcript: str, ollama_url: str, model_name: str) -> str | None:
    try:
        import requests
    except ImportError:
        return None
    try:
        r = requests.post(
            f"{ollama_url.rstrip('/')}/api/generate",
            json={
                "model": model_name,
                "prompt": SUMMARY_PROMPT.format(transcript=transcript[:12000]),
                "stream": False,
            },
            timeout=300,
        )
        r.raise_for_status()
        return r.json().get("response", "").strip() or None
    except Exception as e:  # noqa: BLE001 - summarising is optional
        print(f"  (summary skipped: {e})", file=sys.stderr)
        return None


def main() -> int:
    ap = argparse.ArgumentParser(description="Transcribe NeoClone WAV recordings.")
    ap.add_argument("files", nargs="+", help="WAV files or globs")
    ap.add_argument("--out", default="notes", help="output dir for .md notes")
    ap.add_argument("--model", default="small")
    ap.add_argument("--lang", default=None, help="e.g. en, hi; omit to auto-detect")
    ap.add_argument("--ollama", default="http://localhost:11434")
    ap.add_argument("--ollama-model", default="llama3.2")
    ap.add_argument("--no-summary", action="store_true")
    args = ap.parse_args()

    paths: list[str] = []
    for pat in args.files:
        hits = glob.glob(pat)
        paths.extend(hits if hits else [pat])
    paths = [p for p in paths if os.path.isfile(p)]
    if not paths:
        print("no input files found", file=sys.stderr)
        return 2

    out_dir = Path(args.out)
    out_dir.mkdir(parents=True, exist_ok=True)

    print(f"loading faster-whisper model '{args.model}' (first run downloads)...")
    from faster_whisper import WhisperModel

    model = WhisperModel(args.model, device="auto", compute_type="int8")

    write_summaries = not args.no_summary

    for path in paths:
        print(f"\n=== {path}")
        lines, plain, info = transcribe(path, model, args.lang)
        if not plain:
            print("  (no speech detected)")
            continue

        wav = Path(path)
        stamp = dt.datetime.now().strftime("%Y-%m-%d_%H%M")
        md = out_dir / f"{wav.stem}.md"

        summary = None
        if write_summaries:
            summary = summarize(plain, args.ollama, args.ollama_model)

        with md.open("w", encoding="utf-8") as f:
            f.write(f"# {wav.name}\n\n")
            f.write(f"- source: `{wav}`\n")
            f.write(f"- generated: {stamp}\n")
            f.write(f"- language: {info.language} (p={info.language_probability:.2f})\n")
            f.write(f"- duration: {fmt_ts(info.duration)}\n\n")
            if summary:
                f.write("## Summary\n\n")
                f.write(summary + "\n\n")
            else:
                f.write("_No summary (Ollama not running)._\n\n")
            f.write("## Transcript\n\n")
            f.write("\n".join(lines) + "\n")

        print(f"  -> {md}  ({len(plain.split())} words"
              f"{', summarised' if summary else ''})")

    print("\ndone. grep this folder to search your memory:")
    print(f"  grep -ri 'the thing I forgot' {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
