trivialなDBです。トランザクション処理をします。
Key-Valueで、insert update delete readができます。
いまのところS2PLが動いています。

intel TBBとboostが必要です

### サーバー
'''
make db
./db
'''

### クライアント
は存在しないのでtelnet localhost 31234
'''
$ telnet localhost 31234
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
DB > begin
TransactionID:0
Transaction > insert key value
OK
Transaction > read key
value
Transaction > commit
writing to redo.log
OK!
successfully commited
'''
