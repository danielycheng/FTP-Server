# FTP Server

## Purpose
This program is a bare-bones FTP server that will allow off-the-shelf FTP clients to list, store, and retrieve files from the local file system. Remote procedure calls, remote file systems, file transfer protocol, and Berkeley sockets are used.

FTP exemplifies client-server architecture, and this is a simple server implementation. The protocol is enacted through a pair of TCP connections: (1) a control connection that is accepted by the Server in which the Client provides commands and the Server sends response codes; and (2) separate data connections that are (nominally) accepted by the Client in which data is exchanged (e.g., files are uploaded, downloaded, and directory listings are sent).

The Server implements the bare-bones requirements/commands set forth in Section 5.1 of the FTP specification, IETF RFC 959, with the following exceptions:
- The program does not need to support RECORD structure (only FILE). If the Client attempts to use the “STRU R” command to change from FILE (the default) mode, the Server responds with “504 Command not implemented for that parameter”.
- The program stores and retrieves binary copies of files, i.e., byte-by-byte copies, where a byte is 8-bits. This is the Image type. The program does not support the default file type, ASCII Non-print, which would entail decoding the data back to an internal character representation from the Client- encoded 8-bit NVT-ASCII representation. In other words, this program supports the “TYPE I” command that indicates the Client wishes binary image mode. If a client attempts a STOR or RETR command without having first switched to image mode, the prorgram issues a “451 Requested action aborted: local error in processing” response.

- The Server will only support one active client at a time.
- The Server will interoperate with the default ftp client found on the power nodes (/usr/bin/ftp).
- The “root” of the file namespace serviced by the Server program will be the current working (local) directory when it executed.
