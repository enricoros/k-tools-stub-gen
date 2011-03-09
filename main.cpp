#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QString>

#include "ImplFinder.h"
#include "StubGenerator.h"

#include "pp.h"
#include "CppDocument.h"
#include "FastPreprocessor.h"
#include "Parser.h"
#include "PreprocessorEnvironment.h"

/*
  QCOMPARE(doc->globalSymbolCount(), 1U);
  Class *tst = doc->globalSymbolAt(0)->asClass();
  QCOMPARE(tst->memberCount(), 1U);
  Function *method = tst->memberAt(0)->asFunction();
  QCOMPARE(method->argumentCount(), 1U);
  Argument *arg = method->argumentAt(0)->asArgument();
  QCOMPARE(arg->identifier()->chars(), "arg");
*/

QByteArray readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("header file cannot be found, quitting");
        return QByteArray();
    }
    return file.readAll();
}

QByteArray cppPreprocess(const QByteArray &input, const QString &fileName = "<stdin>")
{
    CPlusPlus::Snapshot snapshot;
    CPlusPlus::FastPreprocessor simplePpClient(snapshot);

    CPlusPlus::Environment environment;
    CPlusPlus::Preprocessor preprocessor(&simplePpClient, &environment);

    QByteArray output;
    preprocessor.preprocess(fileName, input, &output);
    return output;
}

CPlusPlus::Document::Ptr cppParse(const QString &fileName, const QByteArray &preprocData)
{
    CPlusPlus::Document::Ptr doc = CPlusPlus::Document::create(fileName);
    doc->translationUnit()->setSkipFunctionBody(false);
    doc->translationUnit()->setObjCEnabled(false);
    doc->translationUnit()->setCxxOxEnabled(false);
    doc->translationUnit()->setQtMocRunEnabled(false);
    //cppDocument->translationUnit()->blockErrors(false);
    doc->setSource(preprocData);
    if (!doc->parse())
        qWarning("error parsing the preprocessed file");
    doc->check(CPlusPlus::Document::FullCheck);
    if (!doc->diagnosticMessages().isEmpty())
        qWarning() << "info: warnings parsing the file:" << doc->diagnosticMessages().size();
    return doc;
}

int main(int argc, char **args)
{
    if (argc < 2) {
        qWarning("usage: %s somedef.h [someimpl.cpp]", args[0]);
        qWarning("       if an existing implementation is provided, it will be updated");
        return 1;
    }
    QString fileName(args[1]);
    QString outputFile = fileName;
    outputFile.replace(".h", ".cpp");
    if (outputFile == fileName)
        outputFile.append(".cpp");
    if (argc > 2)
        outputFile = QString(args[2]);

    // read input header file
    QByteArray headerData = readFile(fileName);
    if (headerData.isEmpty())
        return 1;

    // add custom definitions (use GUI here)
    headerData.prepend("#define VG_API_CALL \n");
    headerData.prepend("#define VG_API_ENTRY \n");
    headerData.prepend("#define VG_API_EXIT \n");

    // preprocess header
    QByteArray preprocessedHederData = cppPreprocess(headerData);
    if (preprocessedHederData.isEmpty())
        return 1;

    // parse
    CPlusPlus::Document::Ptr headerDocument = cppParse(fileName, preprocessedHederData);
    if (!headerDocument)
        return 1;


    // preprocess and parse output file - IF PRESENT
    CPlusPlus::Document::Ptr cppDocument(0);
    ImplFinder *cppImplFinder = 0;
    if (QFile::exists(outputFile)) {
        QByteArray cppData = readFile(outputFile);
        QByteArray preprocessedCppData = cppPreprocess(cppData);
        cppDocument = cppParse(outputFile, preprocessedCppData);
        cppImplFinder = new ImplFinder(cppDocument->translationUnit());
    }

    // visit the AST of the translation unit with the STUB-GEN
    StubGenerator stubGen(headerDocument->translationUnit());
    stubGen.setImplFinder(cppImplFinder);
    stubGen.setOutputFileName(outputFile);
    stubGen.setVerboseInvocation(true);
    stubGen.generate();
    return 0;
}
