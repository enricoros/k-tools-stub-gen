/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2010 by Enrico Ros <enrico.ros@gmail.com>               *
 *   Started on 19 Apr 2010 by root.
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "StubGenerator.h"
#include "ImplFinder.h"

#include "AST.h"
#include "CoreTypes.h"
#include "Literals.h"
#include "Name.h"
#include "Scope.h"
#include "Symbol.h"
#include "Symbols.h"
#include "TranslationUnit.h"
#include "Type.h"

#include <QDebug>

using namespace CPlusPlus;

StubGenerator::StubGenerator(TranslationUnit *unit)
    : ASTVisitor(unit)
    , m_verboseInvocation(false)
    , m_implFinder(0)
    , m_outputStream(0)
{
}

void StubGenerator::setOutputFileName(const QString &fileName)
{
    m_outputFileName = fileName;
}

QString StubGenerator::outputFileName() const
{
    return m_outputFileName;
}

void StubGenerator::setVerboseInvocation(bool verbose)
{
    m_verboseInvocation = verbose;
}

bool StubGenerator::verboseInvocation() const
{
    return m_verboseInvocation;
}

void StubGenerator::setImplFinder(ImplFinder *implFinder)
{
    m_implFinder = implFinder;
}

bool StubGenerator::generate()
{
    if (m_outputFileName.isEmpty()) {
        qWarning("error: set the file name before calling 'generate'");
        return false;
    }
    QFile outFile(m_outputFileName);
    if (!outFile.open(m_implFinder ? (QIODevice::ReadWrite | QIODevice::Append) : QIODevice::WriteOnly)) {
        qWarning("error: can't open the output file for writing");
        return false;
    }
    m_outputStream = new QTextStream(&outFile);

    // start the visit here, will output all the file
    qWarning() << "input: " << QString(translationUnit()->fileName());
    if (!m_implFinder)
        writeHead();
    accept(translationUnit()->ast());
    if (!m_implFinder)
        writeTail();
    qWarning() << "output:" << m_outputFileName;

    delete m_outputStream;
    m_outputStream = 0;
    return true;
}

bool StubGenerator::visit(FunctionDeclaratorAST *ast)
{
    Function *function = ast->symbol;
    if (!function)
        return true;

    // skip certain types of functions
    if (function->isPureVirtual())
        return true;

    // skip if function already present
    if (m_implFinder && m_implFinder->findFunctionImpl(function))
        return true;

    // write the implementation of the function
    qWarning() << "  writing" << function->identifier()->chars() << "...";
    *m_outputStream << "// " << function->fileName() << " line " << function->line() << "\n";
    writeFunctionSignature(function);
    writeFunctionBody(function);
    return true;
}

void StubGenerator::writeHead()
{
    *m_outputStream << "// this definition stub was generated from the declarations in " << translationUnit()->fileName() << "\n";
    *m_outputStream << "#include \"" << translationUnit()->fileName() << "\"\n";
    if (m_verboseInvocation)
        *m_outputStream << "#include <stdio.h>\n";
    *m_outputStream << "\n";
}

void StubGenerator::writeTail()
{
}

void StubGenerator::writeFunctionSignature(Function *function)
{
    // TODO: missing: template params

    // return type
    if (function->hasReturnType()) {
        writeFullType(function->returnType());
        *m_outputStream << " ";
    }

    // fully-prefixed name
    Symbol *parentSymbol = function->enclosingSymbol();
    for (; parentSymbol; parentSymbol = parentSymbol->enclosingSymbol())
        if (parentSymbol->identifier())
            *m_outputStream << parentSymbol->identifier()->chars() << "::";
    *m_outputStream << function->name()->identifier()->chars();

    // arguments
    *m_outputStream << "(";
    bool firstArg = true;
    for (unsigned int i = 0; i < function->argumentCount(); i++) {
        // comma
        if (!firstArg)
            *m_outputStream << ", ";
        else
            firstArg = false;

        // type
        Argument *arg = function->argumentAt(i)->asArgument();
        if (!arg) {
            qWarning("argument invalid");
            *m_outputStream << "ARG_INVALID";
            continue;
        }
        writeFullType(arg->type());

        // name
        if (arg->name() && arg->name()->identifier()) {
            *m_outputStream << " ";
            *m_outputStream << arg->name()->identifier()->chars();
        }
    }
    *m_outputStream << ")";

    // constness
    if (function->isConst())
        *m_outputStream << " const";
}

void StubGenerator::writeFunctionBody(CPlusPlus::Function *function)
{
    // open function body
    *m_outputStream << "\n";
    *m_outputStream << "{\n";

    // print out params
    if (m_verboseInvocation) {
        *m_outputStream << "    fprintf(stderr, \"STUB ";
        *m_outputStream << function->name()->identifier()->chars();
        *m_outputStream << "(";
        *m_outputStream << ")\\n\");";
        *m_outputStream << "\n";
    }

    // blank line...
    *m_outputStream << "    \n";

    // return value (if needed)
    FullySpecifiedType returnType = function->returnType();
    if (returnType.isValid() && !returnType.type()->isVoidType()) {
        *m_outputStream << "    return ";
        writeReturnPrimitive(returnType.type());
        *m_outputStream << ";\n";
    }

    // close function body
    *m_outputStream << "}\n";
    *m_outputStream << "\n";
}

void StubGenerator::writeFullType(const CPlusPlus::FullySpecifiedType &fulltype)
{
    if (fulltype.isConst())
        *m_outputStream << "const ";
    if (fulltype.isUnsigned())
        *m_outputStream << "unsigned ";
    // TODO: could support more types here
    writeTypePrimitive(fulltype.type());
}

void StubGenerator::writeTypePrimitive(Type *t)
{
    if (t->isUndefinedType()) {
        return;
    } else if (t->isVoidType()) {
        *m_outputStream << "void";
    } else if (t->isIntegerType()) {
        IntegerType *i = t->asIntegerType();
        switch (i->kind()) {
        case IntegerType::Char:
            *m_outputStream << "char";
            break;
        case IntegerType::WideChar:
            *m_outputStream << "char";
            break;
        case IntegerType::Bool:
            *m_outputStream << "bool";
            break;
        case IntegerType::Short:
            *m_outputStream << "short";
            break;
        case IntegerType::Int:
            *m_outputStream << "int";
            break;
        case IntegerType::Long:
            *m_outputStream << "long";
            break;
        case IntegerType::LongLong:
            *m_outputStream << "long long";
            break;
        default:
            qWarning("unknown integer type");
            *m_outputStream << "UNK_INT";
            break;
        }
    } else if (t->isFloatType()) {
        FloatType *f = t->asFloatType();
        switch (f->kind()) {
        case FloatType::Float:
            *m_outputStream << "float";
            break;
        case FloatType::Double:
            *m_outputStream << "double";
            break;
        case FloatType::LongDouble:
            *m_outputStream << "longdouble";
            break;
        }
    } else if (t->isPointerType()) {
        PointerType *p = t->asPointerType();
        writeFullType(p->elementType());
        *m_outputStream << "*";
//    } else if (t->isPointerToMemberType()) {
//        Q_ASSERT(false);
    } else if (t->isReferenceType()) {
        ReferenceType *r = t->asReferenceType();
        writeFullType(r->elementType());
        *m_outputStream << "&";
//    } else if (t->isArrayType()) {
//        Q_ASSERT(false);
    } else if (t->isNamedType()) {
        NamedType *n = t->asNamedType();
        *m_outputStream << n->name()->identifier()->chars();
//    } else if (t->isFunctionType()) {
//        Q_ASSERT(false);
//    } else if (t->isNamespaceType()) {
//        Q_ASSERT(false);
//    } else if (t->isClassType()) {
//        Q_ASSERT(false);
//    } else if (t->isEnumType()) {
//        Q_ASSERT(false);
//    } else if (t->isForwardClassDeclarationType()) {
//        Q_ASSERT(false);
    } else {
        qWarning("unknown type!");
        *m_outputStream << "UNKNOWN";
    }
}

void StubGenerator::writeReturnPrimitive(Type *t)
{
    if (t->isIntegerType()) {
        IntegerType *i = t->asIntegerType();
        if (i->kind() == IntegerType::Bool)
            *m_outputStream << "false";
        else
            *m_outputStream << "0";
    } else if (t->isFloatType() || t->isPointerType() || t->isPointerToMemberType()) {
        *m_outputStream << "0";
    } else if (t->isNamedType()) {
        NamedType *n = t->asNamedType();
        *m_outputStream << n->name()->identifier()->chars();
        *m_outputStream << "()";
    } else {
        qWarning("unknown return type!");
        *m_outputStream << "UNKNOWN";
    }
}
