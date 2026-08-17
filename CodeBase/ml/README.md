# Machine Learning Pipeline

## Purpose

Training, evaluation, and deployment of ML models used in the Ability ecosystem.

## Primary Model: Dual-Stream Bi-LSTM (Hybrid Fusion)

Used by the **Glove Module** to recognize sign language by fusing:
- **Glove stream:** 5 flex values + 4 IMU quaternion values (100+ Hz)
- **Vision stream:** 543 MediaPipe Holistic landmarks (5–8 FPS on RPi Zero 2W)

Captures all **5 linguistic parameters** of sign language (handshape, orientation, movement, location, facial expression).

## Directory Structure

```
ml/
├── datasets/
│   ├── raw/            # Raw recorded sensor + camera data (gitignored — large)
│   └── processed/      # Preprocessed feature tensors ready for training
│
├── models/
│   ├── architectures/  # Model definitions (Bi-LSTM, etc.)
│   └── trained/        # Exported .tflite / .pt files (gitignored — large)
│
├── training/           # Training scripts, hyperparameter configs
│   ├── train.py        # Main training script
│   ├── config.yaml     # Hyperparameters
│   └── augment.py      # Data augmentation utilities
│
└── evaluation/         # Evaluation, confusion matrices, benchmarks
    ├── evaluate.py     # Accuracy, F1, confusion matrix
    └── benchmark.py    # Latency benchmarking on RPi Zero 2W
```

## Workflow

1. **Data Collection:** Record sign language sessions with glove + camera → save to `datasets/raw/`
2. **Preprocessing:** Extract features, align timestamps, create sliding windows → `datasets/processed/`
3. **Training:** Train Bi-LSTM on fused features → export to TFLite → `models/trained/`
4. **Evaluation:** Accuracy, F1 score, confusion matrix, per-class analysis
5. **Deployment:** Copy `.tflite` model to Glove Module's RPi Zero 2W
