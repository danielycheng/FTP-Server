# FTP Server

## Purpose
This program is a bare-bones FTP server that will allow off-the-shelf FTP clients to list, store, and retrieve files from the local file system. Remote procedure calls, remote file systems, file transfer protocol, and Berkeley sockets are used.

FTP exemplifies client-server architecture, and this is a simple server implementation. The protocol is enacted through a pair of TCP connections: (1) a control connection that is accepted by the Server in which the Client provides commands and the Server sends response codes; and (2) separate data connections that are (nominally) accepted by the Client in which data is exchanged (e.g., files are uploaded, downloaded, and directory listings are sent).
