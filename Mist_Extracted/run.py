import sys
import os

# Path to your venv site-packages
venv_path = os.path.join(os.path.dirname(__file__), 'venv', 'Lib', 'site-packages')
if os.path.exists(venv_path):
    sys.path.insert(0, venv_path)
else:
    print("Warning: venv not found! Some packages may fail to import.")

# Import all main dependencies
try:
    import numpy
    import pandas
    import matplotlib
    import flask
    import fastapi
    import jupyter
    import seaborn
    import ipykernel
    import argon2_cffi
    import pyyaml
    import requests
    import httpx
    import werkzeug
    import jsonschema
    import pillow
    import ipywidgets
    import sqlalchemy
    import typing_extensions
    import json5
    import lark
    import pydantic
    import starlette
    import anyio
    import nbformat
    import nbconvert
    # …add anything else from your pip install logs
except ImportError as e:
    print(f"Failed to import: {e}")

print("Python environment loaded successfully!")
