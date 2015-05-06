#ifndef ISPINBOX_H
#define ISPINBOX_H

#include <QWidget>

class ISpinBox : public QWidget
{
    Q_OBJECT
public:
    explicit ISpinBox(QWidget *parent = 0);
    virtual ~ISpinBox();
    virtual void setRange(const double &min, const double& max, const double &step) = 0;
    virtual double value() const = 0;
    virtual double step() const = 0;
    virtual double min() const = 0;
    virtual double max() const = 0;
    virtual QWidget *spinWidget() const = 0;
    virtual QWidget *buttonDownWidget() const = 0;
    virtual QWidget *buttonUpWidget() const = 0;
public slots:
    virtual void setIconDownButton(const QIcon &icon) = 0;
    virtual void setTextDownButton(const QString &text) = 0;
    virtual void setIconUpButton(const QIcon &icon) = 0;
    virtual void setTextUpButton(const QString &text) = 0;

    virtual void setValue(const double &value) = 0;

    virtual void downStep() = 0;
    virtual void upStep() = 0;
signals:
    void downButtonPressed();
    void downButtonReleased();
    void upButtonPressed();
    void upButtonReleased();
    void valueChanged();
};

#endif // ISPINBOX_H
