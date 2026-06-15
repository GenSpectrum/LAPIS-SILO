#!/bin/bash
# Usage: ./queryLocalSilo.sh "query"         — full output
#        ./queryLocalSilo.sh -s "query"      — status code only
if [ "$1" = "-s" ]; then
  curl -s -o /dev/null -w "%{http_code}\n" -X POST http://localhost:8081/query \
    -H 'Content-Type: text/plain' -d "$2"
else
  curl -s -X POST http://localhost:8081/query \
    -H 'Content-Type: text/plain' -w '\n%{http_code}\n' -d "$1"
fi
