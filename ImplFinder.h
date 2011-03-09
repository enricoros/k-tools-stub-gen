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

#ifndef IMPLFINDER_H
#define IMPLFINDER_H

#include "ASTVisitor.h"

class ImplFinder : public CPlusPlus::ASTVisitor
{
public:
    ImplFinder(CPlusPlus::TranslationUnit *unit);

    bool findFunctionImpl(CPlusPlus::Function *function);

protected:
    // ::ASTVisitor
    bool visit(CPlusPlus::FunctionDefinitionAST *);

private:
    CPlusPlus::Function *m_function;
    bool m_found;
};

#endif // IMPLFINDER_H
