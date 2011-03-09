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

#include "ImplFinder.h"
#include "AST.h"
#include "TranslationUnit.h"
#include "CoreTypes.h"
#include "Symbols.h"
#include <QDebug>

using namespace CPlusPlus;

ImplFinder::ImplFinder(CPlusPlus::TranslationUnit *unit)
  : ASTVisitor(unit)
  , m_function(0)
  , m_found(false)
{
}

bool ImplFinder::findFunctionImpl(Function *function)
{
    m_found = false;
    m_function = function;
    accept(translationUnit()->ast());
    m_function = 0;
    return m_found;
}

bool ImplFinder::visit(FunctionDefinitionAST *functionDefAst)
{
    if (functionDefAst->symbol && functionDefAst->symbol->isEqualTo(m_function))
        m_found = true;
    return true;
}
