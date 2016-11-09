#ifndef BLOCKBROWSER_H
#define BLOCKBROWSER_H

#include "clientmodel.h"
#include "main.h"

#include <QDialog>

double getTxTotalValue(std::string);
double convertCoins(int64_t);
int64_t getBlockTime(int);
double getBlockReward(int);
std::string getBlockHash(int);
std::string getBlockMerkle(int);
std::string getBlocknBits(int);
uint64_t getBlockNonce(int);
uint64_t getBlockHashrate(int);
std::string getInputs(std::string);
std::string getOutputs(std::string);
bool addnode(std::string);
const CBlockIndex* getBlockIndex(int);

namespace Ui {
class BlockBrowser;
}
class ClientModel;

class BlockBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit BlockBrowser(QWidget *parent = 0);
    ~BlockBrowser();
	
    void setModel(ClientModel *model);
    
public slots:
    
    void blockClicked();
    void txClicked();
    void updateExplorer(bool);
    double getTxFees(std::string);
	
private slots:

private:
    Ui::BlockBrowser *ui;
    ClientModel *model;
    
};

#endif // BLOCKBROWSER_H
