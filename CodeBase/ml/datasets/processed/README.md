# ML Datasets — Processed

Preprocessed feature tensors ready for model training.

## Expected Format

- `X_train.npy` — Training features, shape: `(num_samples, window_size, num_features)`
- `y_train.npy` — Training labels, shape: `(num_samples,)`
- `X_val.npy` / `y_val.npy` — Validation set
- `X_test.npy` / `y_test.npy` — Test set
- `label_map.json` — Maps integer labels to sign/gesture names
