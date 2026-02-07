.PHONY: test install run clean

install:
	pip install -r requirements.txt

test:
	pytest tests/ -v

test-cov:
	pytest tests/ --cov=src/tiny_interpreter --cov-report=html

run:
	python -m src.tiny_interpreter.main

clean:
	rm -rf __pycache__ .pytest_cache .coverage htmlcov
	find . -type d -name "*.egg-info" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
