#!/bin/bash
trap 'echo traped!' INT
kill -s INT $$
