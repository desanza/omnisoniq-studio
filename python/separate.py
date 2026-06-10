"""
Omnisoniq stem separator — wraps Facebook Demucs.
Usage: python separate.py <input_file> <output_dir>
Outputs progress lines: PROGRESS 0.45
"""
import sys
import os
import subprocess
import re

def main():
    if len(sys.argv) < 3:
        print("Usage: separate.py <input_file> <output_dir>", file=sys.stderr)
        sys.exit(1)

    input_file = sys.argv[1]
    output_dir = sys.argv[2]

    if not os.path.isfile(input_file):
        print(f"ERROR: Input file not found: {input_file}", file=sys.stderr)
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    # Try to import demucs to give a clear error if not installed
    try:
        import demucs  # noqa
    except ImportError:
        print("ERROR: demucs not installed. Run: pip install demucs", file=sys.stderr)
        sys.exit(2)

    # Run demucs as a subprocess to capture its progress output
    cmd = [
        sys.executable, "-m", "demucs",
        "--two-stems", "vocals",   # produces vocals + no_vocals (melody+drums+bass)
        "-n", "htdemucs_6s",       # 6-stem model: drums, bass, guitar, piano, other, vocals
        "-o", output_dir,
        input_file
    ]

    # Fallback to 4-stem if 6-stem model isn't available
    print("PROGRESS 0.02", flush=True)

    proc = subprocess.Popen(
        [sys.executable, "-m", "demucs",
         "-n", "htdemucs",
         "-o", output_dir,
         input_file],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1
    )

    percent_pattern = re.compile(r"(\d+)%")
    reported_pct = 0

    for line in proc.stdout:
        line = line.strip()
        if line:
            # Parse progress from demucs output e.g. "Separated track  45%|..."
            m = percent_pattern.search(line)
            if m:
                pct = int(m.group(1))
                if pct > reported_pct:
                    reported_pct = pct
                    print(f"PROGRESS {pct/100:.2f}", flush=True)

    proc.wait()

    if proc.returncode != 0:
        print("ERROR: demucs exited with error", file=sys.stderr)
        sys.exit(proc.returncode)

    print("PROGRESS 1.00", flush=True)
    print("DONE", flush=True)

if __name__ == "__main__":
    main()
