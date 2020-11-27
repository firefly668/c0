FROM g++:10
WORKDIR /app/
COPY ./* ./
RUN g++ c0.cpp -o c0
RUN chmod +x c0