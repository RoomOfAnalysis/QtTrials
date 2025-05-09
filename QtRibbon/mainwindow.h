﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "SARibbonMainWindow.h"
class SARibbonCategory;
class SARibbonContextCategory;
class SARibbonCustomizeWidget;
class SARibbonActionsManager;
class SARibbonQuickAccessBar;
class SARibbonButtonGroupWidget;
class SARibbonPannel;
class QTextEdit;
class QComboBox;
class QCloseEvent;
class QLineEdit;

class MainWindow: public SARibbonMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* par = nullptr);

private:
    void createRibbonApplicationButton();

    void createCategoryMain(SARibbonCategory* page);
    void createCategoryOther(SARibbonCategory* page);
    void createCategoryDelete(SARibbonCategory* page);
    void createCategorySize(SARibbonCategory* page);
    void createContextCategory1();
    void createContextCategory2();
    void createContextCategoryPage1(SARibbonCategory* page);
    void createContextCategoryPage2(SARibbonCategory* page);
    void createQuickAccessBar();
    void createRightButtonGroup();
    void createWindowButtonGroupBar();

    void createActionsManager();
    QAction* createAction(const QString& text, const QString& iconurl, const QString& objName);
    QAction* createAction(const QString& text, const QString& iconurl);
private Q_SLOTS:
    void onMenuButtonPopupCheckableTest(bool b);
    void onInstantPopupCheckableTest(bool b);
    void onDelayedPopupCheckableTest(bool b);
    void onMenuButtonPopupCheckabletriggered(bool b);
    void onInstantPopupCheckabletriggered(bool b);
    void onDelayedPopupCheckabletriggered(bool b);
    void onShowContextCategory(bool on);
    void onStyleClicked(int id);
    void onActionCustomizeTriggered();
    void onActionCustomizeAndSaveTriggered();
    void onActionCustomizeAndSaveWithApplyTriggered();
    void onActionHelpTriggered();
    void onActionRemoveAppBtnTriggered(bool b);
    void onActionUseQssTriggered();
    void onActionLoadCustomizeXmlFileTriggered();
    void onActionWindowFlagNormalButtonTriggered(bool b);
    void onFontComWidgetCurrentFontChanged(const QFont& f);
    void onActionFontLargerTriggered();
    void onActionFontSmallerTriggered();
    void onActionWordWrapTriggered(bool b);
    void onButtonGroupActionTriggered(QAction* act);
    void onRibbonThemeComboBoxCurrentIndexChanged(int index);
    void onActionHideActionTriggered(bool on);
    void onActionVisibleAllTriggered(bool on);
    void onCheckBoxAlignmentCenterClicked(bool checked);
    //
    void onSpinBoxRibbonTitleHeightChanged(int h);
    void onSpinBoxRibbonTabHeightChanged(int h);
    void onSpinBoxRibbonCategoryHeightChanged(int h);
    void onSpinBoxRibbonPannelTtitleHeightChanged(int h);
    void onSpinBoxRibbonPannelSpacingChanged(int h);
    void onSpinBoxRibbonPannelToolBtnIconSizeChanged(int h);

private:
    void createOtherActions();

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    SARibbonContextCategory* mContextCategory;
    SARibbonContextCategory* mContextCategory2;
    SARibbonCustomizeWidget* mWidgetForCustomize;
    QTextEdit* mTextedit;
    SARibbonActionsManager* mActionsManager;
    int mTagForActionText;
    int mTagForActionIcon;
    QMenu* mMenuApplicationBtn;
    QComboBox* mComboboxRibbonTheme;
    QLineEdit* mSearchEditor{nullptr};
    //
    QAction* mActionWordWrap{nullptr};
    QAction* mActionDisable{nullptr};
    QAction* mActionUnlock{nullptr};
    QAction* mActionSetTextTest{nullptr};
    QAction* mActionShowTest{nullptr};
    QAction* mActionHideAction2{nullptr};
    QAction* mActionHideAction4{nullptr};
    QAction* mActionHideShowTextAct2{nullptr};
    QAction* mActionHideShowTextAct3{nullptr};
    QAction* mActionHideShowTextAct4{nullptr};
    //
    QAction* mOtherAction1{nullptr};
    QAction* mOtherAction2{nullptr};
    QAction* mOtherAction3{nullptr};
    QAction* mOtherAction4{nullptr};
    QAction* mOtherAction5{nullptr};
    QAction* mOtherActionIcon1{nullptr};
    SARibbonPannel* mPannelVisbileExample{nullptr};
    //
    QAction* mActionVisibleAll{nullptr};
    bool mHasApplyCustomizeXmlFile{false};
};

#endif // MAINWINDOW_H
