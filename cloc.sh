#! /bin/sh

echo $(date) \	$(cloc --exclude-dir=vendor,rfc,build,.ccls-cache . | grep 'SUM') >> cloc_log

