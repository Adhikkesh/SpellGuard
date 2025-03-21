FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    qtbase5-dev \
    qtwebengine5-dev \
    libqt5webchannel5-dev \
    libqt5gui5 \
    libqt5widgets5 \
    libqt5core5a \
    qt5-qmake \
    libgl1-mesa-dri \
    libglapi-mesa \
    libgl1-mesa-glx \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    x11-apps \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN rm -f SymSpellChecker *.o *.moc moc_* qrc_* Makefile
RUN qmake "CASE STUDY.pro" && make
ENV DISPLAY=:0
CMD ["./SymSpellChecker", "words_alpha.txt", "english_word_frequency.csv"]