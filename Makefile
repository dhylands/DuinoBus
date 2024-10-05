# DunioBus makefile

THIS_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
TOP_DIR ?= $(THIS_DIR)

DUINO_MAKEFILE ?= $(THIS_DIR)/../DuinoMakefile

ifeq ("$(wildcard $(DUINO_MAKEFILE)/Makefile)","")
$(error Unable to open $(DUINO_MAKEFILE)/Makefile)
else
include $(DUINO_MAKEFILE)/Makefile
endif

PYTHON_FILES = $(shell find . -name '*.py' -not -path  './.direnv/*' -not -path './tests/*')

pystyle:
	yapf -i $(PYTHON_FILES)

pylint:
	pylint $(PYTHON_FILES)

pytest:
	pytest -vv

pycoverage:
	coverage run --source=duino_bus -m pytest
	coverage report -m
