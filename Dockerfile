# Container image that runs your code
FROM alt:p9

RUN apt-get update

RUN apt-get install -y qt5-base-devel cmake qt5-tools-devel qt5-tools libuuid-devel libsmbclient-devel libsasl2-devel catch2-devel doxygen glib2-devel libpcre-devel rpm-build gear libldap-devel libcmocka-devel libkrb5-devel samba-devel

RUN useradd -ms /bin/bash builder && mkdir /app && chown root:builder /app

RUN pwd && ls
RUN cd home && pwd && ls
RUN cd home/builder && pwd && ls
RUN cd home/builder && pwd && cmake .
RUN cd home/builder/app && pwd && ls && cmake .

# Copies your code file from your action repository to the filesystem path `/` of the container
COPY script/build.sh /build.sh

USER builder
WORKDIR /home/builder

# Code file to execute when the docker container starts up (`build.sh`)
# ENTRYPOINT ["/build.sh"]
CMD pwd && ls && cd app && ls && cmake .
