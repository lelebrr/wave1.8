#!/usr/bin/env python3
"""
neura9_trainer.py - Treinador offline da NEURA9 (IA defensiva do WavePwn)

Objetivo:
    - Treinar um classificador leve (72 features -> 10 classes)
    - Exportar modelo .tflite (float32) para uso com TensorFlow Lite Micro
    - Opcionalmente gerar um array C para embutir no firmware

Formato de dataset esperado (CSV):
    - Arquivo: ai/dataset/neura9_dataset.csv
    - Colunas:
        f0, f1, ..., f71, label
    - label é um inteiro 0-9 correspondendo a NEURA9_THREAT_LABELS em
      neura9_inference.cpp / neura9_labels.txt

Uso:
    $ python3 ai/neura9_trainer.py --dataset ai/dataset/neura9_dataset.csv
"""

import argparse
import pathlib

import numpy as np
import tensorflow as tf
from tensorflow import keras


LABELS = [
    "SAFE",
    "CROWDED",
    "OPEN_NETWORK",
    "EVIL_TWIN_RISK",
    "DEAUTH_DETECTED",
    "ROGUE_AP",
    "HIGH_RISK",
    "BATTERY_CRITICAL",
    "GESTURE_COMMAND",
    "LEARNING_MODE",
]


def load_dataset(path: pathlib.Path):
    data = np.loadtxt(path, delimiter=",", skiprows=1)
    x = data[:, :-1].astype("float32")
    y = data[:, -1].astype("int32")
    return x, y


def build_model(input_dim: int = 72, num_classes: int = 10) -> keras.Model:
    inputs = keras.Input(shape=(input_dim,), name="features")
    x = keras.layers.Dense(64, activation="relu")(inputs)
    x = keras.layers.Dense(64, activation="relu")(x)
    x = keras.layers.Dense(32, activation="relu")(x)
    outputs = keras.layers.Dense(num_classes, activation="softmax")(x)
    model = keras.Model(inputs=inputs, outputs=outputs, name="neura9_defense")
    model.compile(
        optimizer=keras.optimizers.Adam(learning_rate=1e-3),
        loss="sparse_categorical_crossentropy",
        metrics=["accuracy"],
    )
    return model


def export_tflite(model: keras.Model, out_path: pathlib.Path):
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    tflite_model = converter.convert()
    out_path.write_bytes(tflite_model)
    print(f"[NEURA9] Modelo TFLite salvo em: {out_path}")


def export_c_array(tflite_bytes: bytes, out_path: pathlib.Path, symbol: str):
    with out_path.open("w") as f:
        f.write('#include "neura9_defense_model_data.h"\n\n')
        f.write("const unsigned char {}[] = {{\n".format(symbol))
        for i, b in enumerate(tflite_bytes):
            if i % 12 == 0:
                f.write("    ")
            f.write(f"0x{b:02X}, ")
            if i % 12 == 11:
                f.write("\n")
        if len(tflite_bytes) % 12 != 0:
            f.write("\n")
        f.write("};\n")
        f.write(
            "const unsigned int {}_len = sizeof({});\n".format(symbol, symbol)
        )
    print(f"[NEURA9] Array C salvo em: {out_path}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--dataset",
        type=pathlib.Path,
        default=pathlib.Path("ai/dataset/neura9_dataset.csv"),
        help="Caminho para o CSV com 72 features + label",
    )
    parser.add_argument(
        "--epochs",
        type=int,
        default=40,
        help="Número de épocas de treinamento",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=256,
        help="Tamanho do batch",
    )
    args = parser.parse_args()

    if not args.dataset.exists():
        raise SystemExit(
            f"Dataset não encontrado em {args.dataset}. "
            "Crie ai/dataset/neura9_dataset.csv com 72 colunas de features + label."
        )

    x, y = load_dataset(args.dataset)
    num_classes = len(np.unique(y))
    if num_classes != 10:
        print(
            f"[NEURA9] Aviso: dataset contém {num_classes} classes distintas "
            "(esperado: 10)"
        )

    model = build_model(input_dim=x.shape[1], num_classes=10)
    model.summary()

    model.fit(
        x,
        y,
        validation_split=0.2,
        epochs=args.epochs,
        batch_size=args.batch_size,
        shuffle=True,
    )

    out_dir = pathlib.Path("ai")
    out_dir.mkdir(parents=True, exist_ok=True)

    tflite_path = out_dir / "neura9_defense_model.tflite"
    export_tflite(model, tflite_path)

    c_array_path = out_dir / "neura9_defense_model_data.cpp"
    export_c_array(
        tflite_path.read_bytes(),
        c_array_path,
        symbol="neura9_defense_model_tflite",
    )


if __name__ == "__main__":
    main()