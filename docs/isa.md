| opc | instr | args   | definition                                     |
|-----|-------|--------|------------------------------------------------|
| 0   | nop   |        |                                                |
|                                                                       |
| 10  | mov   | a,b    | [a]=b                                          |
| 11  | mov   | a,b    | [a]=[b]                                        |
| 12  | mov   | a,b    | [a]=[[b]]                                      |
| 13  | mov   | a,b    | [[a]]=b                                        |
| 14  | mov   | a,b    | [[a]]=[b]                                      |
| 15  | mov   | a,b    | [[a]]=[[b]]                                    |
|                                                                       |
| 20  | jmp   | a      | [0]=a                                          |
| 21  | jmp   | a      | [0]=[a]                                        |
| 22  | jmp   | a      | [0]=[[a]]                                      |
|                                                                       |
| 30  | jez   | a,b    | if [a]==0 [0]=b                                |
| 31  | jez   | a,b    | if [a]==0 [0]=[b]                              |
| 32  | jez   | a,b    | if [a]==0 [0]=[[b]]                            |
| 33  | jez   | a,b    | if [[a]]==0 [0]=b                              |
| 34  | jez   | a,b    | if [[a]]==0 [0]=[b]                            |
| 35  | jez   | a,b    | if [[a]]==0 [0]=[[b]]                          |
|     |       |        |                                                |
| 40  | jgz   | a,b    | if [a]>0 [0]=b                                 |
| 41  | jgz   | a,b    | if [a]>0 [0]=[b]                               |
| 42  | jgz   | a,b    | if [a]>0 [0]=[[b]]                             |
| 43  | jgz   | a,b    | if [[a]]>0 [0]=b                               |
| 44  | jgz   | a,b    | if [[a]]>0 [0]=[b]                             |
| 45  | jgz   | a,b    | if [[a]]>0 [0]=[[b]]                           |
|     |       |        |                                                |
| 50  | jlz   | a,b    | if [a]<0 [0]=b                                 |
| 51  | jlz   | a,b    | if [a]<0 [0]=[b]                               |
| 52  | jlz   | a,b    | if [a]<0 [0]=[[b]]                             |
| 53  | jlz   | a,b    | if [[a]]<0 [0]=b                               |
| 54  | jlz   | a,b    | if [[a]]<0 [0]=[b]                             |
| 55  | jlz   | a,b    | if [[a]]<0 [0]=[[b]]                           |
|     |       |        |                                                |
| 60  | sys   | a,b    | syscall\[a\]([b])                              |
| 61  | sys   | a,b    | syscall\[a\]([[b]])                            |
|     |       |        |                                                |
| 70  | neg   | a      | [a]=-[a]                                       |
| 71  | neg   | a      | [[a]]=-[[a]]                                   |
|     |       |        |                                                |
| 80  | abs   | a      | [a]=abs([a])                                   |
| 81  | abs   | a      | [[a]]=abs([[a]])                               |
|     |       |        |                                                |
| 90  | inc   | a      | [a]++                                          |
| 91  | inc   | a      | [[a]]++                                        |
|     |       |        |                                                |
| 100 | dec   | a      | [a]--                                          |
| 101 | dec   | a      | [[a]]--                                        |
|     |       |        |                                                |
| 110 | add   | a,b    | [a]=[a]+b                                      |
| 111 | add   | a,b    | [a]=[a]+[b]                                    |
| 112 | add   | a,b    | [a]=[a]+[[b]]                                  |
| 113 | add   | a,b    | [[a]]=[[a]]+b                                  |
| 114 | add   | a,b    | [[a]]=[[a]]+[b]                                |
| 115 | add   | a,b    | [[a]]=[[a]]+[[b]]                              |
| 116 | add   | a,b,o  | [a]=[a]+b; if io [o] = 1 el [o] = 0            |
| 117 | add   | a,b,o  | [a]=[a]+[b]; if io [o] = 1 el [o] = 0          |
| 118 | add   | a,b,o  | [a]=[a]+[[b]]; if io [o] = 1 el [o] = 0        |
| 119 | add   | a,b,o  | [[a]]=[[a]]+b; if io [o] = 1 el [o] = 0        |
| 120 | add   | a,b,o  | [[a]]=[[a]]+[b]; if io [o] = 1 el [o] = 0      |
| 121 | add   | a,b,o  | [[a]]=[[a]]+[[b]]; if io [o] = 1 el [o] = 0    |
| 122 | add   | a,b,o  | [a]=[a]+b; if io [[o]] = 1 el [[o]] = 0        |
| 123 | add   | a,b,o  | [a]=[a]+[b]; if io [[o]] = 1 el [[o]] = 0      |
| 124 | add   | a,b,o  | [a]=[a]+[[b]]; if io [[o]] = 1 el [[o]] = 0    |
| 125 | add   | a,b,o  | [[a]]=[[a]]+b; if io [[o]] = 1 el [[o]] = 0    |
| 126 | add   | a,b,o  | [[a]]=[[a]]+[b]; if io [[o]] = 1 el [[o]] = 0  |
| 127 | add   | a,b,o  | [[a]]=[[a]]+[[b]]; if io [[o]] = 1 el [[o]] = 0|
|     |       |        |                                                |
| 130 | div   | a,b    | [a]=[a] div b                                  |
| 131 | div   | a,b    | [a]=[a] div [b]                                |
| 132 | div   | a,b    | [a]=[a] div [[b]]                              |
| 133 | div   | a,b    | [[a]]=[[a]] div b                              |
| 134 | div   | a,b    | [[a]]=[[a]] div [b]                            |
| 135 | div   | a,b    | [[a]]=[[a]] div [[b]]                          |
| 136 | div   | a,b,m  | [a]=[a] div b; [m]=[a] mod b                   |
| 137 | div   | a,b,m  | [a]=[a] div [b]; [m]=[a] mod b                 |
| 138 | div   | a,b,m  | [a]=[a] div [[b]]; [m]=[a] mod b               |
| 139 | div   | a,b,m  | [[a]]=[[a]] div b; [m]=[[a]] mod b             |
| 140 | div   | a,b,m  | [[a]]=[[a]] div [b]; [m]=[[a]] mod b           |
| 141 | div   | a,b,m  | [[a]]=[[a]] div [[b]]; [m]=[[a]] mod b         |
| 142 | div   | a,b,m  | [a]=[a] div b; [[m]]=[a] mod b                 |
| 143 | div   | a,b,m  | [a]=[a] div [b]; [[m]]=[a] mod b               |
| 144 | div   | a,b,m  | [a]=[a] div [[b]]; [[m]]=[a] mod b             |
| 145 | div   | a,b,m  | [[a]]=[[a]] div b; [[m]]=[[a]] mod b           |
| 146 | div   | a,b,m  | [[a]]=[[a]] div [b]; [[m]]=[[a]] mod b         |
| 147 | div   | a,b,m  | [[a]]=[[a]] div [[b]]; [[m]]=[[a]] mod b       |
|     |       |        |                                                |
| 150 | mul   | a,b    | [a]=[a]*b                                      |
| 151 | mul   | a,b    | [a]=[a]*[b]                                    |
| 152 | mul   | a,b    | [a]=[a]*[[b]]                                  |
| 153 | mul   | a,b    | [[a]]=[[a]]*b                                  |
| 154 | mul   | a,b    | [[a]]=[[a]]*[b]                                |
| 155 | mul   | a,b    | [[a]]=[[a]]*[[b]]                              |
|     |       |        |                                                |
| 160 | shl   | a,b    | [a]=[a]<<b                                     |
| 161 | shl   | a,b    | [a]=[a]<<[b]                                   |
| 162 | shl   | a,b    | [a]=[a]<<[[b]]                                 |
| 163 | shl   | a,b    | [[a]]=[[a]]<<b                                 |
| 164 | shl   | a,b    | [[a]]=[[a]]<<[b]                               |
| 165 | shl   | a,b    | [[a]]=[[a]]<<[[b]]                             |
|     |       |        |                                                |
| 170 | shr   | a,b    | [a]=[a]>>b                                     |
| 171 | shr   | a,b    | [a]=[a]>>[b]                                   |
| 172 | shr   | a,b    | [a]=[a]>>[[b]]                                 |
| 173 | shr   | a,b    | [[a]]=[[a]]>>b                                 |
| 174 | shr   | a,b    | [[a]]=[[a]]>>[b]                               |
| 175 | shr   | a,b    | [[a]]=[[a]]>>[[b]]                             |
