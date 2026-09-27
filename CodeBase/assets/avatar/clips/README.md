# Assets — Avatar Animation Clips

Pre-rendered sign language animation clips played on the Deaf Module's screen.

**Video files in `clips/` are gitignored** — too large for Git. Store locally or use Git LFS.

## How It Works

1. Central Hub receives TEXT (e.g., "Hello, how are you?")
2. Hub converts TEXT → sign sequence IDs: `["HELLO", "HOW", "ARE", "YOU", "QUESTION"]`
3. Hub sends sign IDs to Deaf Module (~100 bytes)
4. Deaf Module looks up each ID → plays corresponding clip from this directory

## Expected File Structure

```
clips/
├── HELLO.mp4       (or .gif / .webm)
├── HOW.mp4
├── ARE.mp4
├── YOU.mp4
├── QUESTION.mp4
├── ...
└── alphabet/
    ├── A.mp4
    ├── B.mp4
    └── ...
```
