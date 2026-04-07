@startuml
' ================= 模块1: AST 节点树 =================
package "AST 节点树" as AST {
    Node <|-- NumberNode
    Node <|-- BinaryNode
    Node <|-- UnaryNode
    Node <|-- VariableNode
    BinaryNode <|-- MultiplyNode
    BinaryNode <|-- DivideNode
    BinaryNode <|-- AddNode
    BinaryNode <|-- AssignNode
    BinaryNode <|-- SubtractNode
    UnaryNode <|-- NegateNode
    UnaryNode <|-- FunNode

    abstract class Node {
        + calc(): double
        + isLvalue(): bool
        + assign(double value): void
    }
    abstract class BinaryNode {
        + left_: Node*
        + right_: Node*
    }
    class AssignNode {
        + calc(): double
    }
    class VariableNode {
        + id_: const unsigned int
        + Storage& storage_
        + VariableNode(unsigned int id, Storage& storage)
        --
        + calc(): double
        + isLvalue(): bool
        + assign(double value): void
    }
    class MultiplyNode {
        + childs_: vector<Node*>
        + positives_: vector<bool>
        --
        + addChild(Node* child, bool positive): void
    }
}

' ================= 模块2: 序列化与数据管理 =================
package "序列化与数据管理" as SER {
    Serializable --> Serializer
    Serializable --> DeSerializer
    Serializable <|-- Calc
    Serializable <|-- Storage
    Serializable <|-- SymbolTable

    class Serializable {
        + serialize(Serializer& serializer): void
        + deserialize(DeSerializer& deserializer): void
    }
    class Serializer {
        +stream_: ofstream
        --
        + operator<<(unsigned int): Serializer&
        + operator<<(int): Serializer&
        + operator<<(unsigned long): Serializer&
        + operator<<(long): Serializer&
        + operator<<(double): Serializer&
        + operator<<(const string&): Serializer&
        + operator<<(bool): Serializer&
    }
    class DeSerializer {
        +stream_: ifstream
        --
        + operator>>(unsigned int&): DeSerializer&
        + operator>>(int&): DeSerializer&
        + operator>>(unsigned long&): DeSerializer&
        + operator>>(long&): DeSerializer&
        + operator>>(double&): DeSerializer&
        + operator>>(string&): DeSerializer&
        + operator>>(bool&): DeSerializer&
    }
    class SymbolTable {
        + dictionary_: map<string, unsigned int>
        + curId_: unsigned int
        --
        + add(const string& str): unsigned int
        + find(const string& str): unsigned int
        + clear(): void
        + getSymbolName(unsigned int id): string
    }
    class Storage {
        + cells_: vector<double>
        + inits_: vector<bool>
        --
        + Storage(SymbolTable& tbl)
        + clear(): void
        + addConstant(SymbolTable& tbl): void
        + isInit(unsigned int id): bool
        + getValue(unsigned int id): double
        + setValue(unsigned int id, double value): void
        + addValue(unsigned int id, double value): void
    }
    class Calc {
        - symTbl_: SymbolTable
        - storage_: Storage
        --
        - getStorage(): Storage&
        - addSymbol(const string& str): unsigned int
        - findSymbol(const string& str): unsigned int
    }
}

' ================= 模块3: 词法与语法分析 =================
package "词法与语法分析" as PAR {
    class EToken <<enumeration>> {
        + TOKEN_END
        + TOKEN_ERROR
        + TOKEN_NUMBER
        + TOKEN_PLUS
        + TOKEN_MINUS
        + TOKEN_MULTIPLY
        + TOKEN_DIVIDE
        + TOKEN_LPAREN
        + TOKEN_RPAREN
        + TOKEN_IDENTIFIER
    }
    class Scanner {
        + buf_: string
        + number_: double
        + curPos_: unsigned int
        + token_: EToken
        --
        + accept():void
        + number():double
        + token():EToken
        - skipWhiteSpace():void
    }
    class EStatus <<enumeration>> {
        + STATUS_OK
        + STATUS_ERROR
        + STATUS_QUIT
    }
    class Parser {
        - tree_: Node*
        - scanner_: Scanner&
        - status_: EStatus
        --
        + parse():void
        + calculate():double
        - expr(): Node*
        - term(): Node*
        - factor(): Node*
    }
}
@enduml