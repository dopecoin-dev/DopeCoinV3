#include "blockbrowser.h"
#include "ui_blockbrowser.h"
#include "main.h"
#include "util.h"
#include "bitcoinrpc.h"
#include "base58.h"
#include "clientmodel.h"
#include "db.h"

using namespace std;

double GetPoSKernelPS(const CBlockIndex* blockindex);
double GetDifficulty(const CBlockIndex* blockindex);

uint64_t getBlockHashrate(int height)
{
	const CBlockIndex* pindex = getBlockIndex(height);
	
    double timeDiff = getBlockTime(height) - getBlockTime(1);
    double timePerBlock = timeDiff / height;

    return (boost::uint64_t)(((double)GetDifficulty(pindex) * pow(2.0, 32)) / timePerBlock);
}

const CBlockIndex* getBlockIndex(int height)
{
    std::string hex = getBlockHash(height);
    uint256 hash(hex);
    return mapBlockIndex[hash];
}

int64_t getBlockTime(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nTime;
}

double getBlockReward(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return ValueFromAmount(pblockindex->nMint).get_real();
}

std::string getBlockHash(int Height)
{
    if(Height > pindexBest->nHeight) { return ""; }
    if(Height < 0) { return ""; }
    int64 desiredheight;
    desiredheight = Height;
    if (desiredheight < 0 || desiredheight > nBestHeight)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hashBestChain];
    while (pblockindex->nHeight > desiredheight)
        pblockindex = pblockindex->pprev;
    return  pblockindex->GetBlockHash().GetHex(); // pblockindex->phashBlock->GetHex();
}

std::string getBlockMerkle(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->hashMerkleRoot.ToString();//.substr(0,10).c_str();
}

uint64_t getBlockNonce(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->nNonce;
}

std::string getBlocknBits(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return HexBits(pblockindex->nBits);
}

std::string getBlockDebug(int Height)
{
    std::string strHash = getBlockHash(Height);
    uint256 hash(strHash);

    if (mapBlockIndex.count(hash) == 0)
        return 0;

    CBlock block;
    CBlockIndex* pblockindex = mapBlockIndex[hash];
    return pblockindex->ToString();
}

double getTxTotalValue(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return -1;

    CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << tx;

    double value = 0;
    double buffer = 0;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        buffer = value + convertCoins(txout.nValue);
        value = buffer;
    }

    return value;
}

double convertCoins(int64_t amount)
{
    return (double)amount / (double)COIN;
}

std::string getOutputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "N/A";

    std::string str = "";
    for (unsigned int i = (tx.IsCoinStake() ? 1 : 0); i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];

        CTxDestination address;
        if (!ExtractDestination(txout.scriptPubKey, address) )
            address = CNoDestination();

        double buffer = convertCoins(txout.nValue);
        std::string amount = boost::to_string(buffer);
        str.append(CBitcoinAddress(address).ToString());
        str.append(": ");
        str.append(amount);
        str.append(" DOPE");
        str.append("\n");
    }

    return str;
}

std::string getInputs(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    if (!GetTransaction(hash, tx, hashBlock))
        return "N/A";

    std::string str = "";
    for (unsigned int i = 0; i < tx.vin.size(); i++)
    {
        uint256 hash;
        const CTxIn& vin = tx.vin[i];

        hash.SetHex(vin.prevout.hash.ToString());
        CTransaction wtxPrev;
        uint256 hashBlock = 0;
        if (!GetTransaction(hash, wtxPrev, hashBlock))
             return "N/A";

        CTxDestination address;
        if (!ExtractDestination(wtxPrev.vout[vin.prevout.n].scriptPubKey, address) )
            address = CNoDestination();

        double buffer = convertCoins(wtxPrev.vout[vin.prevout.n].nValue);
        std::string amount = boost::to_string(buffer);
        str.append(CBitcoinAddress(address).ToString());
        str.append(": ");
        str.append(amount);
        str.append(" DOPE");
        str.append("\n");
    }

    return str;
}

double BlockBrowser::getTxFees(std::string txid)
{
    uint256 hash;
    hash.SetHex(txid);

    CTransaction tx;
    uint256 hashBlock = 0;
    CTxDB txdb("r");

    if (!GetTransaction(hash, tx, hashBlock))
        return -1;

    MapPrevTx mapInputs;
    map<uint256, CTxIndex> mapUnused;
    bool fInvalid;

    if (!tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
        return -1;

    int64 nTxFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();

    if(tx.IsCoinStake() || tx.IsCoinBase()) {
        ui->feesLabel->setText(QString(tr("Reward:")));
        nTxFees *= -1;
    }
    else
        ui->feesLabel->setText(QString(tr("Fees:")));

    return convertCoins(nTxFees);
}

BlockBrowser::BlockBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BlockBrowser)
{
    ui->setupUi(this);

//    setFixedSize(400, 420);

    connect(ui->blockButton, SIGNAL(pressed()), this, SLOT(blockClicked()));
    connect(ui->txButton, SIGNAL(pressed()), this, SLOT(txClicked()));
}

void BlockBrowser::updateExplorer(bool block)
{    
    if (block)
    {
        int height = ui->heightBox->value();
		
        if (height > pindexBest->nHeight)
        {
            ui->heightBox->setValue(pindexBest->nHeight);
            height = pindexBest->nHeight;
        }

		const CBlockIndex* pindex = getBlockIndex(height);
		
        ui->heightBox2->setText(QString::number(height));
		ui->rewardBox->setText(QString::number(getBlockReward(height), 'f', 3) + " DOPE");
		ui->hashBox->setText(QString::fromUtf8(getBlockHash(height).c_str()));
        ui->merkleBox->setText(QString::fromUtf8(getBlockMerkle(height).c_str()));
        ui->bitsBox->setText(QString::fromUtf8(getBlocknBits(height).c_str()));
        ui->nonceBox->setText(QString::number(getBlockNonce(height)));
        ui->timeBox->setText(QString::fromUtf8(DateTimeStrFormat(getBlockTime(height)).c_str()));		
        ui->hardBox->setText(QString::number(GetDifficulty(pindex), 'f', 6));
		
		if (pindex->IsProofOfStake()) {
            ui->pawLabel->setText(QString(tr("Block Network Stake Weight:")));
            ui->hardLabel->setText(QString(tr("Block Difficulty (POS):")));
            ui->pawBox->setText(QString::number(GetPoSKernelPS(pindex), 'f', 3) + " ");
        }
        else {
            ui->pawLabel->setText(QString(tr("Block Hashrate:")));
            ui->hardLabel->setText(QString(tr("Block Difficulty (POW):")));
            ui->pawBox->setText(QString::number(getBlockHashrate(height)/1000000, 'f', 3) + " MH/s");			
		}
    } 
    
    else {
        std::string txid = ui->txBox->text().toUtf8().constData();
		
        double value = getTxTotalValue(txid);
        double fees = getTxFees(txid);

        ui->valueBox->setText(value == -1.0 ? "N/A" : (QString::number(value, 'f', 6) + " DOPE"));
        ui->txID->setText(value == -1.0 ? "N/A" : (QString::fromUtf8(txid.c_str())));
        ui->outputBox->setText(QString::fromUtf8(getOutputs(txid).c_str()));
        ui->inputBox->setText(QString::fromUtf8(getInputs(txid).c_str()));
        ui->feesBox->setText(fees == -1.0 ? "N/A" : (QString::number(fees, 'f', 6) + " DOPE"));
		
    }
}

void BlockBrowser::txClicked()
{
    updateExplorer(false);
}

void BlockBrowser::blockClicked()
{
    updateExplorer(true);
}

void BlockBrowser::setModel(ClientModel *model)
{
    this->model = model;
}

BlockBrowser::~BlockBrowser()
{
    delete ui;
}
