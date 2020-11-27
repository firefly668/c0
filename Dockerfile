FROM alpine:3
RUN apk add cmake gcc g++ libgcc build-base make --no-cache
WORKDIR /app/
COPY ./* ./
RUN g++ c0.cpp -o c0
RUN chmod +x c0