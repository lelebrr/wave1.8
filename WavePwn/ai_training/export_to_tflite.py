#!/usr/bin/env python3
"""
export_to_tflite.py - Converte um modelo Keras (.h5) para TensorFlow Lite (.tflite)

Uso:

    python export_to_tflite.py --model best_model.h5 --output neura9_defense_model.tflite

Por convenção, execute a partir da pasta WavePwn/:

    cd WavePwn
    python ai_training/export_to_tflite.py \
        --model ai_training/best_model.h5 \
        --output ai/neura9_defense_model.tflite
"""

import argparse
import pathlib

import tensorflow as tf


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--model",
        type=pathlib.Path,
        required=True,
        help="Caminho para o modelo Keras (.h5)",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        required=True,
        help="Caminho de saída para o .tflite",
    )
    args = parser.parse_args()

    if not args.model.exists():
        raise SystemExit(f"Modelo não encontrado em {args.model}")

    print(f"[NEURA9] Carregando modelo Keras de {args.model}")
    model = tf.keras.models.load_model(args.model)

    print("[NEURA9] Convertendo para TensorFlow Lite (float32)")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    tflite_model = converter.convert()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(tflite_model)

    print(f"[NEURA9] Modelo TFLite salvo em: {args.output}")


if __name__ == "__main__":
    main()