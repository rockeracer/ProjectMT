import os
import re
import glob
from dataclasses import dataclass, field

class ParseError(Exception):
    def __init__(self, ErrorMsg):
        self.ErrorMsg = ErrorMsg

    ErrorMsg = ''

@dataclass
class VariableData:
    Name : str = ''
    Type : str = ''
    bConst : bool = False
    bPointer : bool = False
    bReference : bool = False
    bTemplate : bool = False
    TemplateType : str = ''
    bTemplateTypePointer : bool = False

@dataclass
class PropertyData:
    Variable : VariableData = VariableData()
    Attributes : list = field(default_factory=list)
    Access : str = 'private'

@dataclass
class FunctionData:
    Name : str = ''
    Attributes : list = field(default_factory=list)
    bVirtual : bool = False
    bStatic : bool = False
    bConst : bool = False
    bPure : bool = False
    bOverride : bool = False
    ReturnType : VariableData = VariableData()
    Params : list = field(default_factory=list)
    Access : str = 'private'

@dataclass
class ClassData:
    Name : str = ''
    Parent : list = field(default_factory=list)
    Attributes : list = field(default_factory=list)
    Functions : list = field(default_factory=list)
    Properties: list = field(default_factory=list)


class HeaderParser:
    def IsWhiteSpace(self, char):
        return char == ' ' or char == '\n' or char == '\t'

    def Token(self):
        Token = ''
        while 1:
            Cursor = self.HeaderFile.tell()
            Char = self.HeaderFile.read(1)
            
            if not self.IsWhiteSpace(Char):
                if re.match('[a-zA-Z0-9_]', Char):
                    Token += Char
                else:
                    if Token:
                        self.HeaderFile.seek(Cursor)
                    else:
                        Token = Char

                    break;
            else:
                if Token:
                    break

        return Token

    def Parse(self, HeaderFilePath):
        self.HeaderFile = open(HeaderFilePath, 'r')

        if not self.HeaderFile:
            raise ParseError('File not found!')
            return
        
        while 1:
            Token = self.Token()
            if Token:
                if Token == 'MTCLASS':
                    self.Classes.append(self.ParseClass())
            else:
                break

        self.HeaderFile.close()

    def ParseClass(self):
        Data = ClassData()

        Data.Attributes = self.ParseBlock('(', ')')
        
        Token = self.Token()
        if Token != 'class':
            raise ParseError('class keyword is required!')
            return

        Data.Name = self.Token()
        
        Token = self.Token()
        if Token == ':':
            while 1:
                Token = self.Token()
                if Token == 'public' or Token == 'protected' or Token == 'private':
                    Data.Parent.append(self.Token())
                else:
                    Data.Parent.append(Token)

                Token = self.Token()
                if Token == '{':
                    break
                elif Token != ',':
                    raise ParseError(', is required!')
                    return
        elif Token != '{':
            raise ParseError('{ is required!')
            return

        self.StartBlock('{', '}')
        Access = 'private'
        while 1:
            Token = self.BlockToken()
            if not Token:
                break
            if Token == 'public' or Token == 'protected' or Token == 'private':
                Access = Token
            elif Token == 'MTPROPERTY':
                Property = self.ParseProperty()
                Property.Access = Access
                Data.Properties.append(Property)
            elif Token == 'MTFUNCTION':
                Function = self.ParseFunction()
                Function.Access = Access
                Data.Functions.append(Function)

        return Data

    def ParseProperty(self):
        Data = PropertyData()
        PropertyAttribute = self.ParseBlock('(', ')')
        if PropertyAttribute:
            Data.Attributes = PropertyAttribute.split(',')

        Data.Variable, EndChar = self.ParseVariable([';'])

        if Data.Variable.bConst:
            raise ParseError('Property Should NOT const! (%s)' % Data.Variable.Name)
        if Data.Variable.bReference:
            raise ParseError('Property Should NOT Reference! (%s)' % Data.Variable.Name)
        if Data.Variable.bTemplate and Data.Variable.bPointer:
            raise ParseError('Template property Should NOT Pointer! (%s)' % Data.Variable.Name)

        return Data

    def ParseVariable(self, EndCharList):
        Data = VariableData()

        Token = self.Token()
        
        if Token == 'const':
            Data.bConst = True
            Token = self.Token()

        Data.Type = Token
        
        if Data.Type == 'TArray' or Data.Type == 'TMap':
            Data.bTemplate = True
            Data.TemplateType = self.ParseBlock('<', '>')

            if Data.TemplateType.count('*') > 1:
                raise ParseError('Template type Should NOT double pointer!')
                return

            if Data.TemplateType[-1] == '*':
                Data.bTemplateTypePointer = True
                Data.TemplateType = Data.TemplateType[:-1]

        while 1:
            Token = self.Token()

            if Token == '*':
                Data.bPointer = True
            elif Token == '&':
                Data.bReference = True
            elif Token == 'const':
                Data.bConst = True
            else:
                break

        Data.Name = Token

        while 1:
            Token = self.Token()
            if Token in EndCharList:
                break

        return Data, Token

    def ParseFunction(self):
        Data = FunctionData()

        FunctionAttribute = self.ParseBlock('(', ')')
        if FunctionAttribute:
            Data.Attributes = FunctionAttribute.split(',')

        Cursor = self.HeaderFile.tell()
        Token = self.Token()

        if Token == 'static':
            Data.bStatic = True
            Cursor = self.HeaderFile.tell()
            Token = self.Token()
        elif Token == 'virtual':
            Data.bVirtual = True
            Cursor = self.HeaderFile.tell()
            Token = self.Token()

        if Token == 'void':
            Data.Name = self.Token()
            if self.Token() != '(':
                raise ParseError('Function %s Parse Error : ( is missing!' % Data.Name)
        else:
            self.HeaderFile.seek(Cursor)
            ReturnType, EndChar = self.ParseVariable(['('])
            Data.ReturnType = ReturnType
            Data.Name = ReturnType.Name

        Cursor = self.HeaderFile.tell()
        Token = self.Token()
        if Token != ')':
            self.HeaderFile.seek(Cursor)
            while 1:
                Param, EndChar = self.ParseVariable([',', ')'])
                Data.Params.append(Param)

                if EndChar == ')':
                    break

        Token = self.Token()

        if Token == 'const':
            Data.bConst = True
            Token = self.Token()

        if Token == '=':
            Token = self.Token()
            if Token != '0':
                raise ParseError('Function %s Parse Error : 0 for Pure is missing!' % Data.Name)

            Data.bPure = True
            Token = self.Token()
        elif Token == 'override':
            Data.bOverride = True
            Token = self.Token()

        if Token != ';':
            raise ParseError('Function %s Parse Error : ; is missing!' % Data.Name)

        return Data



    def ParseBlock(self, Open, Close):
        Block = ''
        InnerBlockCount = 0

        Token = self.Token()
        if Token != Open:
            raise ParseError('%s is required!' % Open)
            return

        while 1:
            Token= self.Token()

            if Token == Open:
                InnerBlockCount += 1
            elif Token == Close:
                if InnerBlockCount > 0:
                    InnerBlockCount -= 1
                else:
                    break

            Block += Token

        return Block

    def StartBlock(self, Open, Close):
        self.StartBlockChar = Open
        self.EndBlockChar = Close
        self.InnerBlockCount = 0

    def BlockToken(self):
        Token = self.Token()

        if Token == self.StartBlockChar:
            self.InnerBlockCount += 1
            return Token
        elif Token == self.EndBlockChar:
            if self.InnerBlockCount > 0:
                self.InnerBlockCount -= 1
                return Token
            else:
                return None
        else:
            return Token

    def GenerateHeaderFile(self, FilePath):
        if not self.Classes:
            return

        GeneratedHeaderFile = open(FilePath, 'w')
        GeneratedHeaderFile.write('#pragma once\n\n')

        for Class in self.Classes:
            GeneratedHeaderFile.write('static auto ClassRegister_%s = MTClassRegister<class %s>("%s");\n\n' % (Class.Name, Class.Name, Class.Name))
            GeneratedHeaderFile.write('#define REGISTER_CLASS_%s()\\\n' % Class.Name)
            GeneratedHeaderFile.write('\tstatic void RegisterProperty(MTClass* Class)\\\n')
            GeneratedHeaderFile.write('\t{\\\n')

            for Property in Class.Properties:
                Type = Property.Variable.Type
                if Property.Variable.bPointer:
                    Type += '*'
                elif Property.Variable.bTemplate:
                    if Property.Variable.bTemplateTypePointer:
                        Type += '<%s*>' % Property.Variable.TemplateType
                    else:
                        Type += '<%s>' % Property.Variable.TemplateType
                GeneratedHeaderFile.write('\t\tClass->AddProperty("%s", "%s", &%s::%s);\\\n' % (Type, Property.Variable.Name, Class.Name, Property.Variable.Name))

            GeneratedHeaderFile.write('\t}\n')

        GeneratedHeaderFile.close()

    HeaderFile = None

    StartBlockChar = ''
    EndBlockChar = ''
    InnerBlockCount = 0

    Classes = []

try:
    ModuleFilelist = glob.glob('../../**/*.build.json', recursive=True)
    for ModuleFile in ModuleFilelist:
        ModuleFileDir = os.path.dirname(ModuleFile)
        ModuleName = os.path.split(ModuleFile)[1].split('.')[0]
        
        HeaderFilelist = glob.glob(ModuleFileDir + '/**/*.h', recursive=True)
        for h in HeaderFilelist:
            print(h)
    #p = HeaderParser()
    #p.Parse('../../Test/TestClass.h')
    #p.GenerateHeaderFile('../../Test/TestClass.gen.h')
    
except ParseError as e:
    print(e.ErrorMsg)

os.system('pause')
