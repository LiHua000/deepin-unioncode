// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LLGENERATOR_H
#define LLGENERATOR_H

#include "llglobal.h"
#include "services/language/languagegenerator.h"
#include "services/terminal/terminalservice.h"
#include "services/debugger/debuggerservice.h"

class LLGenerator : public dpfservice::LanguageGenerator
{
    Q_OBJECT
public:
    explicit LLGenerator();
    virtual ~LLGenerator() override;

    static QString toolKitName() { return LL_TOOLKIT; }

    //disabled debug function for now
    QString debugger() override;
    bool prepareDebug(const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool requestDAPPort(const QString &ppid, const QMap<QString, QVariant> &param, QString &retMsg) override;
    bool isNeedBuild() override;
    bool isTargetReady() override;
    bool isLaunchNotAttach() override;
    dap::LaunchRequest launchDAP(const QMap<QString, QVariant> &param) override;
    QString build(const QString &projectPath) override;
    QString getProjectFile(const QString &projectPath) override;
    QMap<QString, QVariant> getDebugArguments(const dpfservice::ProjectInfo &projectInfo,
                                              const QString &currentFile) override;

    dpfservice::RunCommandInfo getRunArguments(const dpfservice::ProjectInfo &projectInfo,
                                               const QString &currentFile) override;
private:
    dpfservice::TerminalService *terminalSrv { nullptr };
    dpfservice::DebuggerService *debuggerSrv { nullptr };
};

#endif   // LLGENERATOR_H
