#!/bin/bash

# Need to add this so that the %packager macro used in .spec
# is defined, actual value doesn't matter since this is just
# for testing
echo "%packager Docker <docker@email.com>" >> ~/.rpmmacros

cd /app/ && gear-rpm -ba
