# ML Training Scripts

## Files to Create

| File | Purpose |
|---|---|
| `train.py` | Main training script — loads data, trains Bi-LSTM, saves model |
| `config.yaml` | Hyperparameters (learning rate, batch size, epochs, window size) |
| `augment.py` | Data augmentation (time warping, noise injection, random rotation) |
| `preprocess.py` | Raw data → processed features (alignment, windowing, normalization) |
