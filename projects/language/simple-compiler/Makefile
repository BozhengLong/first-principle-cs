.PHONY: test install run clean test-cov compile

install:
	pip install -r requirements.txt

test:
	pytest tests/ -v

test-cov:
	pytest tests/ --cov=src/simple_compiler --cov-report=html

compile:
	python -m src.simple_compiler.main $(FILE)

run: compile
	python -m src.simple_compiler.vm $(FILE:.sl=.bc)

clean:
	rm -rf __pycache__ .pytest_cache .coverage htmlcov
	find . -type d -name "*.egg-info" -exec rm -rf {} +
	find . -type f -name "*.pyc" -delete
	find . -type f -name "*.bc" -delete
