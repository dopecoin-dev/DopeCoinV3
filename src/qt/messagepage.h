#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include <QWidget>

namespace Ui {
    class MessagePage;
}
class MessageModel;
//class OptionsModel;

QT_BEGIN_NAMESPACE
class QTableView;
class QItemSelection;
class QSortFilterProxyModel;
class QMenu;
class QModelIndex;
class MessageViewDelegate;
QT_END_NAMESPACE


/** Widget that shows a list of sending or receiving addresses.
  */
class MessagePage : public QWidget
{
    Q_OBJECT

public:

    explicit MessagePage(QWidget *parent = 0);
    ~MessagePage();

    void setModel(MessageModel *model);

private:
    void setupTextActions();

public slots:
    void exportClicked();

private:
    Ui::MessagePage *ui;
    MessageModel *model;
    
    QMenu *contextMenu;
    QMenu *contextMenu2;
    QAction *replyAction;
    QAction *deleteAction;
    QAction *copyAction;
    QString replyFromAddress;
    QString replyToAddress;
    MessageViewDelegate *msgdelegate;

private slots:
    void on_sendButton_clicked();
    void on_newButton_clicked();
    void on_deleteButton_clicked();
    void on_backButton_clicked();
    void copy_clicked();
    void messageTextChanged();
    void selectionChanged();
    void itemSelectionChanged();
    void incomingMessage();
    /** Spawn contextual menu (right mouse menu) for address book entry */
    void contextualMenu(const QPoint &point);
    void contextualMenu2(const QPoint &point);

signals:
};

#endif // MESSAGEPAGE_H
