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

#ifndef STUBGENERATOR_H
#define STUBGENERATOR_H

#include "ASTVisitor.h"
#include <QFile>
#include <QTextStream>
class ImplFinder;

class StubGenerator : public CPlusPlus::ASTVisitor
{
public:
    StubGenerator(CPlusPlus::TranslationUnit *unit);

    void setOutputFileName(const QString &fileName);
    QString outputFileName() const;

    void setVerboseInvocation(bool verbose);
    bool verboseInvocation() const;

    void setImplFinder(ImplFinder *);

    bool generate();

protected:
    // ::ASTVisitor
    bool visit(CPlusPlus::FunctionDeclaratorAST *ast);

private:
    void writeHead();
    void writeTail();
    void writeFunctionSignature(CPlusPlus::Function *);
    void writeFunctionBody(CPlusPlus::Function *);
    void writeFullType(const CPlusPlus::FullySpecifiedType &);
    void writeTypePrimitive(CPlusPlus::Type *);
    void writeReturnPrimitive(CPlusPlus::Type *);

    QString m_outputFileName;
    bool m_verboseInvocation;
    ImplFinder *m_implFinder;
    QTextStream *m_outputStream;
};

#endif // STUBGENERATOR_H
