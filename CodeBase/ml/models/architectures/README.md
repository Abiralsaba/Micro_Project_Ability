# ML Model Architectures

Model definition code for the Bi-LSTM and any experimental architectures.

## Files to Create

| File | Purpose |
|---|---|
| `bilstm_fusion.py` | Dual-stream Bi-LSTM — glove branch (128-d) + vision branch (256-d) + late fusion + softmax |
| `single_stream.py` | Ablation baseline — single-stream LSTM (glove only or vision only) |
