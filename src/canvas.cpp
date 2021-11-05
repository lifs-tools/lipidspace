#include "lipidspace/canvas.h"


PointSet::PointSet(int _len, Table *_table){
    len = _len;
    table = _table;
    points = new QPointF[len];
}

PointSet::~PointSet(){
    delete []points;
}


Canvas::Canvas(QWidget *parent) : QWidget(parent){
    mousePressed = false;
    antialiased = false;
    scaling = 1.;
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    lipid_space = 0;
    basescale = 1;
    numTiles = 0;
    tileColumns = 0;
    offset.setX(0);
    offset.setY(0);
    basescale = -1;
    oldSize.setX(width());
    oldSize.setX(height());
    color_counter = 0;
    alpha = 128;
}


Canvas::~Canvas(){
    for (auto pointSet : pointSets) delete pointSet;
}



const double Canvas::PRECESION_FACTOR = 10000.;
const vector<QColor> Canvas::COLORS{QColor("#1f77b4"), QColor("#ff7f0e"), QColor("#2ca02c"), QColor("#d62728"), QColor("#9467bd"), QColor("#8c564b"), QColor("#e377c2"), QColor("#7f7f7f"), QColor("#bcbd22"), QColor("#17becf")};




void Canvas::mousePressEvent(QMouseEvent *event){
    if (!mousePressed){
        mousePressed = true;
        deltaMouse = event->pos();
        oldOffset = QPoint(offset.x(), offset.y());
    }
    
}


void Canvas::mouseReleaseEvent(QMouseEvent *){
    mousePressed = false;
}


void Canvas::mouseMoveEvent(QMouseEvent *event){
    if (mousePressed){
        offset.setX(oldOffset.x() + (event->pos().x() - deltaMouse.x()));
        offset.setY(oldOffset.y() + (event->pos().y() - deltaMouse.y()));
        update();
    }
}




    
void Canvas::wheelEvent(QWheelEvent *event){
    
    
    QPoint mouse = event->pos();
    int bound_num = -1;
    for (int i = 0; i < (int)pointSets.size(); ++i){
        PointSet *pointSet = pointSets[i];
        QRectF &bound = pointSet->bound;
        if (bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width() && bound.x() <= mouse.x() && mouse.x() <= bound.x() + bound.width()){
            bound_num = i;
            break;
        }
            
    }
    if (bound_num < 0) return;
    
    double old_scaling = scaling;
    if (event->angleDelta().y() < 0){
        if (scaling > 0.2) scaling /= 1.1;
    }
    else {
        scaling *= 1.1;
    }
    QRectF &bound = pointSets[bound_num]->bound;
    double mx = mouse.x() - bound.x();
    double my = mouse.y() - bound.y();
    offset.setX(mx + (offset.x() - mx) / old_scaling * scaling);
    offset.setY(my + (offset.y() - my) / old_scaling * scaling);
    
    update();
}



void Canvas::setLipidSpace(LipidSpace *_lipid_space){
    lipid_space = _lipid_space;
}



void Canvas::resizeEvent(QResizeEvent* ){
    if (!lipid_space || !pointSets.size()) return;
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_space = (double)width() * MARGIN;
    double h_space = (double)height() * MARGIN;
    double w_rect = (double)width() * ((1. - MARGIN * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - MARGIN * ((double)tileRows + 1.)) / (double)tileRows);
    
    
    double dx = w_space;
    double dy = h_space;
    for (int i = 0, cc = 0; i < numTiles; ++i){
        pointSets[i]->bound.setRect(dx, dy, w_rect, h_rect);
        dx += w_space + w_rect;
        if (++cc >= tileColumns){
            dx = w_space;
            dy += h_space + h_rect;
            cc = 0;
        }
    }
    
    
    double ww = pointSets[0]->bound.width();
    double hh = pointSets[0]->bound.height();
    
    double mx = pointSets[0]->bound.x() + (ww * 0.5);
    double my = pointSets[0]->bound.y() + (hh * 0.5);
    
    
    if (oldSize.x() > 0 && oldSize.y() > 0){
        offset.setX(offset.x() + (mx - oldSize.x()));
        offset.setY(offset.y() + (my - oldSize.y()));
    }
    
    oldSize.setX(mx);
    oldSize.setY(my);
    
    
    update();
}



void Canvas::resetCanvas(){
    
    colorMap.clear();
    color_counter = 0;
    numTiles = 0;
    tileColumns = 0;
    for (auto pointSet : pointSets) delete pointSet;
    pointSets.clear();
    oldSize.setX(0);
    oldSize.setY(0);
    update();
}




void Canvas::refreshCanvas(){
    if (!lipid_space) return;
    
    colorMap.clear();
    color_counter = 0;
    Table* global_lipidome = lipid_space->global_lipidome;
    numTiles = 1 + lipid_space->lipidomes.size();
    tileColumns = ceil(sqrt((double)numTiles));
    
    
    if (!global_lipidome) return;
    
    pointSets.clear();
    
    double x_min = 0;
    double x_max = 0;
    double y_min = 0;
    double y_max = 0;
    if (global_lipidome->m.cols >= 2){
        pointSets.push_back(new PointSet(global_lipidome->m.rows, global_lipidome));
        for (int r = 0; r < global_lipidome->m.rows; ++r){
            double xval = global_lipidome->m(r, 0) * PRECESION_FACTOR;
            double yval = global_lipidome->m(r, 1) * PRECESION_FACTOR;
            x_min = min(x_min, xval);
            x_max = max(x_max, xval);
            y_min = min(y_min, yval);
            y_max = max(y_max, yval);
            pointSets.back()->points[r].setX(xval);
            pointSets.back()->points[r].setY(yval);
            //pointSets.back()->table->intensities[r] = global_lipidome->intensities[r];
        }
    }
    else {
        return;
    }
    x_min *= 1.1;
    x_max *= 1.1;
    y_min *= 1.1;
    y_max *= 1.1;
    
    
    for (Table* lipidome : lipid_space->lipidomes){
        pointSets.push_back(new PointSet(lipidome->m.rows, lipidome));
        for (int r = 0; r < lipidome->m.rows; ++r){
            pointSets.back()->points[r].setX(lipidome->m(r, 0) * PRECESION_FACTOR);
            pointSets.back()->points[r].setY(lipidome->m(r, 1) * PRECESION_FACTOR);
            //pointSets.back()->intensities[r] = lipidome->intensities[r];
        }
    }
    minMax.setRect(x_min, x_max, y_min, y_max);
    
    
    if ((x_max - x_min) / (double)width() > (y_max - y_min) / (double)height()){
        basescale = (x_max - x_min) / (double)width();
    }
    else {
        basescale = (y_max - y_min) / (double)height();
    }
    
    
    int tileRows = ceil((double)numTiles / (double)tileColumns);
    double w_rect = (double)width() * ((1. - MARGIN * ((double)tileColumns + 1.)) / (double)tileColumns);
    double h_rect = (double)height() * ((1. - MARGIN * ((double)tileRows + 1.)) / (double)tileRows);
    
    double ox = w_rect * 0.5 - ((x_max + x_min) / 2 / basescale);
    double oy = h_rect * 0.5 - ((y_max + y_min) / 2 / basescale);
    offset.setX(ox);
    offset.setY(oy);
    
    
    
    resizeEvent(0);
}

    
void Canvas::paintEvent(QPaintEvent *event){
    const QRect & rect = event->rect();
    QPainter painter(this);
    painter.setBrush(brush);
    painter.eraseRect(rect);
    painter.setRenderHint(QPainter::Antialiasing);
    
    if (!pointSets.size()) return;

            
    for (auto pointSet : pointSets){
        for (int i = 0; i < pointSet->len; ++i){
            painter.save();
            
            // setting clipping area
            painter.setClipRect(pointSet->bound);
            
            
            // transform area to according section
            painter.translate(offset);
            painter.translate(QPointF(pointSet->bound.x(), pointSet->bound.y()));
            
            // checking lipid class and selecting color
            string lipid_class = pointSet->table->classes[i];
            if (uncontains_val(colorMap, lipid_class)){
                colorMap.insert({lipid_class, COLORS[color_counter++ % COLORS.size()]});
            }
            
            double intens = pointSet->table->intensities[i] > 1 ? log(pointSet->table->intensities[i]) : 0.5; 
            
            painter.scale(scaling / basescale, scaling / basescale);
            
            
            // setting up pen for painter
            QPen pen;
            QColor qcolor = colorMap[lipid_class];
            qcolor.setAlpha(alpha);
            pen.setColor(qcolor);
            pen.setWidth(7. * intens * basescale);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            
            painter.drawPoint(pointSet->points[i]);
            painter.restore();
        }
    }
    
    
    
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    
    for (auto pointSet : pointSets) painter.drawRect(pointSet->bound);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
