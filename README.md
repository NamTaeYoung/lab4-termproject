# Lab4 Term Project: Chat and File Transfer Messenger

## 프로젝트 개요
- 이 프로젝트는 다중 사용자 채팅 및 파일 전송 기능을 제공하는 GUI 메신저입니다.
- 서버-클라이언트 구조와 GTK 기반 GUI 클라이언트를 사용하여 구현되었습니다.

## 주요 기능
1. 사용자 등록 및 채팅방 생성.
2. 텍스트 기반 채팅 기능.
3. 파일 전송 (FTP 방식 참고).
4. GUI 클라이언트 (GTK 사용).

## 실행 방법

### 서버 실행
```bash
gcc server.c -o server -lpthread
./server

### 클라이언트 실행
gcc chat_client_gui.c -o chat_client_gui `pkg-config --cflags --libs gtk+-3.0`
./chat_client_gui
