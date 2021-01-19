#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <math.h>

#include "NormalizationLib.h"
#include "DispLib.h"
#include "histograms.h"
#include "gradient.h"
#include "RegionU16Lib.h"
#include "StringFcLib.h"


#include <tiffio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace boost;
using namespace std;
using namespace boost::filesystem;
using namespace cv;

//=============================================================================
//------------------------------------------------------------------------------------------------------------------------------
//           Out of Calss functions
//------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//          constructor Destructor
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//          CLASS FUNCTIONS
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::OpenImageFolder()
{
    path ImageFolder( ui->lineEditImageFolder->text().toStdWString());

    ui->listWidgetImageFiles->clear();
    for (directory_entry& FileToProcess : directory_iterator(ImageFolder))
    {
        regex FilePattern(ui->lineEditRegexImageFile->text().toStdString());
        if (!regex_match(FileToProcess.path().filename().string().c_str(), FilePattern ))
            continue;
        path PathLocal = FileToProcess.path();
        if (!exists(PathLocal))
        {
            ui->textEditOut->append(QString::fromStdString(PathLocal.filename().string() + " File not exists" ));
            break;
        }
        ui->listWidgetImageFiles->addItem(PathLocal.filename().string().c_str());
    }
    if(ui->listWidgetImageFiles->count() && ui->checkBoxAutoProcessFirstFile->checkState())
        ui->listWidgetImageFiles->setCurrentRow(0);
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ReadImage()
{
    if(ui->checkBoxAutocleanOut->checkState())
        ui->textEditOut->clear();
    int flags;
    //if(ui->checkBoxLoadAnydepth->checkState())
    //    flags = CV_LOAD_IMAGE_ANYDEPTH;
    //else
    //    flags = IMREAD_COLOR;
    flags = IMREAD_COLOR;
    ImIn = imread(FileToOpen.string(), flags);
    if(ImIn.empty())
    {
        ui->textEditOut->append("improper file");
        return;
    }
    string extension = FileToOpen.extension().string();

    if((extension == ".tif" || extension == ".tiff") && ui->checkBoxShowTiffInfo->checkState())
        ui->textEditOut->append(QString::fromStdString(TiffFilePropetiesAsText(FileToOpen.string())));

    if(ui->checkBoxShowMatInfo->checkState())
        ui->textEditOut->append(QString::fromStdString(MatPropetiesAsText(ImIn)));
    //ProcessImages();
}
//------------------------------------------------------------------------------------------------------------------------------

void MainWindow::ShowsScaledImage(Mat Im, string ImWindowName, double displayScale)
{
    if(Im.empty())
    {
        ui->textEditOut->append("Empty Image to show");
        return;
    }
    Mat ImToShow;

    ImToShow = Im.clone();

    if (displayScale != 1.0)
        cv::resize(ImToShow,ImToShow,Size(), displayScale, displayScale, INTER_AREA);
    imshow(ImWindowName, ImToShow);
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowImages()
{
    if(ImIn.empty())
        return;

    double scale = 1 / ui->doubleSpinBoxImageScale->value();

    if(ui->checkBoxShowInput->checkState())
    {
        ShowsScaledImage(ImIn, "Input Image", scale);
    }
    else
    {
        destroyWindow("Input Image");
    }
/*
    Mat ImToShow;
    ShowSolidRegionOnImage(LesionMask * 3, ImIn).copyTo(ImToShow);;
    rectangle(ImToShow, Rect(tilePositionX,tilePositionY, tileSizeX, tileSizeY), Scalar(0.0, 255.0, 0.0, 0.0),
              10);
    ui->widgetImageWhole->paintBitmap(ImToShow);
    ui->widgetImageWhole->repaint();
*/
}
//------------------------------------------------------------------------------------------------------------------------------
void MainWindow::ProcessImages()
{

}


//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------
//          Slots
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_pushButtonOpenImageFolder_clicked()
{
    QFileDialog dialog(this, "Open Folder");
    //dialog.setFileMode(QFileDialog::Directory);
    //dialog.setDirectory(ui->lineEditImageFolder->text());
    path ImageFolder;
    if(dialog.exec())
    {
        ImageFolder = dialog.directory().path().toStdWString();
    }
    else
        return;
    if (!exists(ImageFolder))
    {
        ui->textEditOut->append(" Image folder : " + QString::fromStdWString(ImageFolder.wstring())+ " not exists ");
        return;
    }
    if (!is_directory(ImageFolder))
    {
        ui->textEditOut->append(" Image folder : " + QString::fromStdWString(ImageFolder.wstring())+ " This is not a directory path ");
        return;
    }
    ui->lineEditImageFolder->setText(QString::fromStdWString(ImageFolder.wstring()));

    OpenImageFolder();
}

void MainWindow::on_listWidgetImageFiles_currentRowChanged(int currentRow)
{
    FileToOpen = ui->lineEditImageFolder->text().toStdWString();
    FileToOpen.append(ui->listWidgetImageFiles->item(currentRow)->text().toStdWString());
    if(!exists(FileToOpen))
    {
        ui->textEditOut->append(" file : " + QString::fromStdWString(FileToOpen.wstring())+ " not exists ");
        return;
    }

    ReadImage();
    ProcessImages();
    ShowImages();
}

void MainWindow::on_pushButtonProcessAllFiles_clicked()
{
    int filesCount = ui->listWidgetImageFiles->count();
    int firstFile = 0;
    ui->textEditOut->clear();
    //time_t begin,end;
    //time (&begin);
    for(int fileNr = firstFile; fileNr< filesCount; fileNr++)
    {
        FileToOpen = ui->lineEditImageFolder->text().toStdWString();
        FileToOpen.append(ui->listWidgetImageFiles->item(fileNr)->text().toStdWString());
        if(!exists(FileToOpen))
        {
            ui->textEditOut->append(" file : " + QString::fromStdWString(FileToOpen.wstring())+ " not exists ");
            return;
        }

        ReadImage();
        ProcessImages();
        ShowImages();
        waitKey(200);

    }
    //time (&end);
    //double difference = difftime (end,begin);
    //QString TimeStringQ = " calcTime = " + QString::number(difference) + " s" + "\n";
    //ui->textEditOut->append(TimeStringQ);
}

void MainWindow::on_checkBoxShowInput_toggled(bool checked)
{
    ShowImages();
}

void MainWindow::on_doubleSpinBoxImageScale_valueChanged(double arg1)
{
    ShowImages();
}
