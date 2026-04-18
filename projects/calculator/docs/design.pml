'  参考： https://www.bilibili.com/video/BV16b4y1J7Dp

@startuml
' ================= 模块1: AST 节点树 =================
package "AST 节点树" as AST {
    Node <|-- NumberNode
    Node <|-- BinaryNode
    Node <|-- UnaryNode
    Node <|-- VariableNode
    Node <|-- NaryNode
    BinaryNode <|-- MultiplyNode
    BinaryNode <|-- DivideNode
    BinaryNode <|-- AddNode
    BinaryNode <|-- AssignNode
    BinaryNode <|-- SubtractNode
    UnaryNode <|-- NegateNode
    UnaryNode <|-- FunNode
    NaryNode <|-- SumNode
    NaryNode <|-- ProductNode

    abstract class Node {
        + calc(): double
        + isLvalue(): bool
        + assign(double value): void
    }
    abstract class BinaryNode {
        - left_: unique_ptr<Node>
        - right_: unique_ptr<Node>
    }
    abstract class UnaryNode {
        # child_: unique_ptr<Node>
    }
    class NumberNode {
        - value_: const double
        + NumberNode(double value)
        + calc(): double
    }
    class VariableNode {
        - symbol_: string
        - env_: Env&
        + VariableNode(string symbol, Env& env)
        + calc(): double
        + isLvalue(): bool
        + assign(double value): void
    }
    class AddNode {
        + calc(): double
    }
    class SubtractNode {
        + calc(): double
    }
    class MultiplyNode {
        + calc(): double
    }
    class DivideNode {
        + calc(): double
    }
    class AssignNode {
        + calc(): double
    }
    class NegateNode {
        + NegateNode(unique_ptr<Node> child)
        + calc(): double
    }
    class FunNode {
        - pfunc_: FuncPtr
        + FunNode(unique_ptr<Node> child, FuncPtr pfunc)
        + calc(): double
    }
    abstract class NaryNode {
        # children_: vector<unique_ptr<Node>>
        # appendChild(unique_ptr<Node> child): void
    }
    class SumNode {
        - operations_: vector<EAdditiveOp>
        + SumNode(unique_ptr<Node> child)
        + addTerm(unique_ptr<Node> term, EAdditiveOp op): void
        + calc(): double
    }
    class ProductNode {
        - operations_: vector<EMultiplicativeOp>
        + ProductNode(unique_ptr<Node> child)
        + addFactor(unique_ptr<Node> factor, EMultiplicativeOp op): void
        + calc(): double
    }
}

' ================= 模块2: 异常处理 =================
package "异常处理" as EXC {
    std::exception <|-- CalcException
    CalcException <|-- SyntaxError
    CalcException <|-- RuntimeError
    RuntimeError <|-- DivisionByZeroError
    RuntimeError <|-- UndefinedVariableError
    RuntimeError <|-- UninitializedVariableError
    SyntaxError <|-- UnknownFunctionError
    SyntaxError <|-- InvalidTokenError

    class CalcException {
        # message_: string
        - stackTraceCache_: mutable string
        --
        + CalcException(const string& message)
        + what(): const char*
        + stackTrace(): const char*
        - captureStackTrace(): static string
    }
    class SyntaxError {
        + SyntaxError(const string& message)
    }
    class RuntimeError {
        + RuntimeError(const string& message)
    }
    class DivisionByZeroError {
        + DivisionByZeroError()
    }
    class UndefinedVariableError {
        + UndefinedVariableError(const string& name)
    }
    class UninitializedVariableError {
        + UninitializedVariableError(const string& name)
    }
    class UnknownFunctionError {
        + UnknownFunctionError(const string& name)
    }
    class InvalidTokenError {
        + InvalidTokenError(char ch)
    }
}

' ================= 模块3: 序列化与数据管理 =================
package "序列化与数据管理" as SER {
    Serializable --> Serializer
    Serializable --> DeSerializer
    Serializable <|-- Env
    Serializable <|-- Storage
    Serializable <|-- SymbolTable

    class Serializable {
        + serialize(Serializer& output): void
        + deserialize(DeSerializer& input): void
    }
    class Serializer {
        - ofs_: ofstream
        --
        + Serializer(const string& filename)
        + put<T>(const T& value): Serializer&
        + putString(const string& value): Serializer&
        + putBool(bool value): Serializer&
        + operator<<(const T& value): Serializer&
    }
    class DeSerializer {
        - ifs_: ifstream
        --
        + DeSerializer(const string& filename)
        + get<T>(T& value): DeSerializer&
        + getString(string& value): DeSerializer&
        + getBool(bool& value): DeSerializer&
        + operator>>(T& value): DeSerializer&
    }
    class SymbolTable {
        - dictionary_: map<string, unsigned int>
        - currentId_: unsigned int
        + kInvalidSymbolId: static constexpr unsigned int
        --
        + add(const string& name): unsigned int
        + find(const string& name): unsigned int
        + clear(): void
        + getSymbolName(unsigned int id): string
        + currentId(): unsigned int
    }
    class Storage {
        - cells_: vector<double>
        - inits_: vector<bool>
        --
        + Storage(SymbolTable& tbl)
        + clear(): void
        + isInit(unsigned int id): bool
        + getValue(unsigned int id): double
        + setValue(unsigned int id, double value): void
        + addConstant(SymbolTable& tbl, const string& name, double value): void
    }
    class FuncTable {
        - funcs_: unique_ptr<FuncPtr[]>
        - size_: unsigned int
        --
        + FuncTable(SymbolTable& tbl)
        + getFunc(unsigned int id): FuncPtr
        + size(): unsigned int
    }
    class Env {
        - symTbl_: SymbolTable
        - funcTbl_: FuncTable
        - storage_: Storage
        --
        + Env()
        + getStorage(): Storage&
        + findFunc(const string& name): FuncPtr
        + addSymbol(const string& name): unsigned int
        + findSymbol(const string& name): unsigned int
        + listVariables(): void
        + listFunctions(): void
    }
}

' ================= 模块4: 词法与语法分析 =================
package "词法与语法分析" as PAR {
    class EToken <<enumeration>> {
        TOKEN_COMMAND
        TOKEN_END
        TOKEN_ERROR
        TOKEN_NUMBER
        TOKEN_PLUS
        TOKEN_MINUS
        TOKEN_MULTIPLY
        TOKEN_DIVIDE
        TOKEN_LPAREN
        TOKEN_RPAREN
        TOKEN_IDENTIFIER
        TOKEN_ASSIGN
    }
    class EStatus <<enumeration>> {
        STATUS_SUCCESS
        STATUS_ERROR
        STATUS_QUIT
    }
    class Scanner {
        - input_: istream&
        - value_: double
        - symbol_: string
        - curPos_: int
        - isEmpty_: bool
        - token_: EToken
        --
        + Scanner(istream& input)
        + accept(): void
        + acceptCommand(): void
        + getValue(): double
        + getSymbol(): const string&
        + getToken(): EToken
        + isEmpty(): bool
        + isDone(): bool
        + isCommand(): bool
        - readChar(): void
    }
    class Parser {
        - ownedBuilder_: unique_ptr<IAstBuilder>
        - ownedEnv_: unique_ptr<Env>
        - builder_: IAstBuilder&
        - scanner_: Scanner&
        - env_: Env&
        - tree_: unique_ptr<Node>
        - status_: EStatus
        --
        + Parser(Scanner& scanner)
        + Parser(Scanner& scanner, IAstBuilder& builder)
        + Parser(Scanner& scanner, Env& env)
        + Parser(Scanner& scanner, IAstBuilder& builder, Env& env)
        + parse(): EStatus
        + calc(): double
        - expr(): unique_ptr<Node>
        - term(): unique_ptr<Node>
        - factor(): unique_ptr<Node>
    }
}

' ================= 模块5: AST 构建器 =================
package "AST 构建器" as BLD {
    interface IAstBuilder {
        + makeNumber(double value): unique_ptr<Node>
        + makeVariable(string symbol, Env& env): unique_ptr<Node>
        + makeFunction(unique_ptr<Node> child, FuncPtr func): unique_ptr<Node>
        + makeNegate(unique_ptr<Node> child): unique_ptr<Node>
        + makeAssign(unique_ptr<Node> left, unique_ptr<Node> right): unique_ptr<Node>
        + makeAdditive(unique_ptr<Node> first, vector<AdditivePart> rest): unique_ptr<Node>
        + makeMultiplicative(unique_ptr<Node> first, vector<MultiplicativePart> rest): unique_ptr<Node>
    }
    class BinaryAstBuilder {
        + makeNumber(double value): unique_ptr<Node>
        + makeVariable(string symbol, Env& env): unique_ptr<Node>
        + makeFunction(unique_ptr<Node> child, FuncPtr func): unique_ptr<Node>
        + makeNegate(unique_ptr<Node> child): unique_ptr<Node>
        + makeAssign(unique_ptr<Node> left, unique_ptr<Node> right): unique_ptr<Node>
        + makeAdditive(unique_ptr<Node> first, vector<AdditivePart> rest): unique_ptr<Node>
        + makeMultiplicative(unique_ptr<Node> first, vector<MultiplicativePart> rest): unique_ptr<Node>
    }
    class NaryAstBuilder {
        + makeNumber(double value): unique_ptr<Node>
        + makeVariable(string symbol, Env& env): unique_ptr<Node>
        + makeFunction(unique_ptr<Node> child, FuncPtr func): unique_ptr<Node>
        + makeNegate(unique_ptr<Node> child): unique_ptr<Node>
        + makeAssign(unique_ptr<Node> left, unique_ptr<Node> right): unique_ptr<Node>
        + makeAdditive(unique_ptr<Node> first, vector<AdditivePart> rest): unique_ptr<Node>
        + makeMultiplicative(unique_ptr<Node> first, vector<MultiplicativePart> rest): unique_ptr<Node>
    }
    IAstBuilder <|.. BinaryAstBuilder
    IAstBuilder <|.. NaryAstBuilder
}

' ================= 模块6: 命令解析器 =================
package "命令解析器" as CMD {
    class CommandParser {
        - scanner_: Scanner&
        - env_: Env&
        - command_: ECommand
        - commandstr_: string
        --
        + CommandParser(Scanner& scanner, Env& env)
        + execute(): EStatus
        - help(): void
        - listVariables(): void
        - listFunctions(): void
        - load(const string& filename): EStatus
        - save(const string& filename): EStatus
    }
    class ECommand <<enumeration>> {
        CMD_HELP
        CMD_LIST_VARS
        CMD_LIST_FUNCS
        CMD_LOAD
        CMD_SAVE
        CMD_QUIT
        CMD_UNKNOWN
    }
}

' ================= 关系 =================
Parser --> Scanner
Parser --> IAstBuilder
Parser --> Env
Parser --> Node
VariableNode --> Env
FunNode --> FuncTable
Env --> SymbolTable
Env --> Storage
Env --> FuncTable
Storage --> SymbolTable
FuncTable --> SymbolTable
CommandParser --> Scanner
CommandParser --> Env
@enduml
