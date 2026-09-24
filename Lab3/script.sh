#!/bin/bash

./app "$@"
EXIT_CODE=$?

if [ $# -eq 2 ]; then
    LEN1=${#1}
    LEN2=${#2}
    EXPECTED=$(( LEN1 - LEN2 ))
    if [ $EXPECTED -lt 0 ]; then
        EXPECTED=$(( -EXPECTED ))
    fi
else
    EXPECTED=0 
fi

if [ $EXIT_CODE -eq $EXPECTED ]; then
    echo "Успіх! Код виходу ($EXIT_CODE) відповідає очікуванням."
else
    echo "Помилка! Програма повернула код ($EXIT_CODE), а скрипт очікував ($EXPECTED)."
fi
