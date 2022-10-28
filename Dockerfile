FROM alpine:3.16 AS builder
RUN apk update && apk add g++ libmilter-dev make
COPY ./ /work
WORKDIR /work
RUN make -j16

FROM alpine:3.16
RUN apk update && apk add libstdc++ libmilter
COPY --from=builder /work/main.out /

CMD ["/main.out"]
