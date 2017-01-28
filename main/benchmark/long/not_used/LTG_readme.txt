LTG Format

Each line describes an LTG

<output> = Threshold value <input1> = weight1 <input2> = weight2 <input3> = weight3 ...

For example:
1216GAT(430) = 3 197GAT(28) = 2 1120GAT(410) = 1 

Describe an LTG whose only one output wire is 1216GAT(430) (or can be viewed as the gate name), and whose input wires (or input gates) are 197GAT(28) and 1120GAT(410)
The threshold value of this LTG is 3
The corresponding weight of the input connecting to 197GAT(28) is 2
The corresponding weight of the input connecting to 1120GAT(410) is 1

Note:

1.The LTG network is constucted by connecting the wire names (gate names)
For example:
A = 2 B = 1 C = 1
E = 3 A = 1 D = 2
F = 2 A = 1 E = 1

In this case:
B and C connect to the inputs of A, D connects to the input of E
A connects to the inputs of E and F
E connects to the input of F

2.If you want to know which ones are the primary inputs and primary outputs, you have to extract them by yourself 
For example:
A = 2 B = 1 C = 1
E = 3 A = 1 D = 2
F = 2 A = 1 E = 1

in this case 
B, C, D are PIs
F is PO
A, E are internal nodes

