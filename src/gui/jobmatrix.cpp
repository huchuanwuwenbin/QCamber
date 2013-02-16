#include "jobmatrix.h"
#include "ui_jobmatrix.h"
#include "structuredtextparser.h"
#include "context.h"
#include "QDebug"
#include <QtGui>

extern Context ctx;

using namespace std;

JobMatrix::JobMatrix(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::JobMatrix)
{
  ui->setupUi(this);
}

JobMatrix::~JobMatrix()
{
  delete ui;
}

void JobMatrix::on_CloseButton_clicked()
{
  this->close();
}

void JobMatrix::SetMatrix(StructuredTextDataStore* ds)
{
  QGridLayout *matrix_layout = new QGridLayout;
  layerSignalMapper = new QSignalMapper(this);
  stepSignalMapper = new  QSignalMapper(this);
  QString text;
  int steps,layers;

  StructuredTextDataStore::BlockIterPair ip = ds->getBlocksByKey("STEP");
  steps = 2;
  for (StructuredTextDataStore::BlockIter it = ip.first; it != ip.second; ++it)
  {
    myLabel *label = new myLabel("this");
    label->setText((QString)it->second->get("NAME").c_str());
    matrix_layout->addWidget(label,0,steps++);
    step_name.append((QString)it->second->get("NAME").c_str());

    connect(label, SIGNAL(clicked()),stepSignalMapper, SLOT(map()));
    stepSignalMapper->setMapping(label,(QString)it->second->get("NAME").c_str());
  }

  ip = ds->getBlocksByKey("LAYER");
  layers = 1;
  for (StructuredTextDataStore::BlockIter it = ip.first; it != ip.second; ++it)
  {
    QLabel *label = new QLabel();        
    text = (QString)it->second->get("TYPE").c_str();
    if(text == "SILK_SCREEN")
      text = "(ss ,";
    else if(text == "SOLDER_MASK")
      text = "(sm ,";
    else if(text == "SIGNAL")
      text = "(sig,";
    else if(text == "DRILL")
      text = "(srl,";
    else if(text == "DOCUMENT")
      text = "(doc,";
    else if(text == "ROUT")
      text = "(rt ,";
    else if(text == "SOLDER_PASTE")
      text = "(sp ,";
    else
      text = "( ,";
    if(it->second->get("POLARITY") == "POSITIVE")
      text += "p)  ";
    else
      text += "n)  ";
    layer_name.append((QString)it->second->get("NAME").c_str());
    text += (QString)it->second->get("NAME").c_str();
    label->setText(text);
    matrix_layout->addWidget(label,layers++,0);

    for(int i=2;i<steps;i++)
    {
      text = step_name[i-2] + "/" + (QString)it->second->get("NAME").c_str();
      QPushButton *btn = new QPushButton(text);
      connect(btn, SIGNAL(clicked()),layerSignalMapper, SLOT(map()));
      layerSignalMapper->setMapping(btn,text);


      text = "steps/";
      text += step_name[i-2].toAscii().data() ;
      text += "/layers/";
      text += (QString)it->second->get("NAME").c_str();
      text += "/features";
      if(GetFileLength(text) == 0)
      {
        //btn->setFlat(true);
        btn->setText("");
      }

      matrix_layout->addWidget(btn,layers-1,i);

    }

  }
  connect(stepSignalMapper, SIGNAL(mapped (const QString &)), this, SLOT(showStep(const QString &)));
  connect(layerSignalMapper, SIGNAL(mapped (const QString &)), this, SLOT(showLayer(const QString &)));
  ui->scrollWidget->setLayout(matrix_layout);

}

unsigned long JobMatrix::GetFileLength ( QString fileName)
{
  QString path = ctx.loader->absPath(fileName).toLower();
  QFile file(path);
  //qDebug()<<ctx.loader->absPath(fileName).toLower();
  if ( file.exists() ) {
    return file.size();
  }else{
    file.setFileName(path+".Z");
    if(file.exists())
      return file.size();
    file.setFileName(path+".z");
    return file.size();
  }
  return -1; //error
}


void JobMatrix::showLayer(const QString feature_name)
{
  QStringList name = feature_name.toLower().split("/");
  QString path = "steps/" + name[0] + "/layers/" + name[1] + "/features";
  path = ctx.loader->absPath(path.toLower());
  QFile file(path);
  widget.clear_scene();
  widget.load_profile(name[0]);
  if (file.exists()) {
    widget.load_feature(path);
  } else {
    widget.load_feature(path + ".Z");
  }
  widget.show();
}

void JobMatrix::showStep(const QString step_name)
{
  Window.setWindowTitle(step_name);
  Window.addLayerLabel(layer_name);
  Window.show();
}
