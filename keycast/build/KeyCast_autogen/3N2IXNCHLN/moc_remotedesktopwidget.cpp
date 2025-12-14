/****************************************************************************
** Meta object code from reading C++ file 'remotedesktopwidget.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/desktop/remotedesktopwidget.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'remotedesktopwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN19RemoteDesktopWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto RemoteDesktopWidget::qt_create_metaobjectdata<qt_meta_tag_ZN19RemoteDesktopWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "RemoteDesktopWidget",
        "keyPressed",
        "",
        "key",
        "Qt::KeyboardModifiers",
        "modifiers",
        "keyReleased",
        "mousePressed",
        "x",
        "y",
        "Qt::MouseButton",
        "button",
        "mouseReleased",
        "mouseMoved",
        "mouseDoubleClicked",
        "wheelScrolled",
        "delta",
        "setConnected",
        "connected",
        "updateFrame",
        "QImage",
        "frame",
        "clear"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'keyPressed'
        QtMocHelpers::SignalData<void(int, Qt::KeyboardModifiers)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'keyReleased'
        QtMocHelpers::SignalData<void(int, Qt::KeyboardModifiers)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'mousePressed'
        QtMocHelpers::SignalData<void(int, int, Qt::MouseButton)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 }, { 0x80000000 | 10, 11 },
        }}),
        // Signal 'mouseReleased'
        QtMocHelpers::SignalData<void(int, int, Qt::MouseButton)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 }, { 0x80000000 | 10, 11 },
        }}),
        // Signal 'mouseMoved'
        QtMocHelpers::SignalData<void(int, int)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 },
        }}),
        // Signal 'mouseDoubleClicked'
        QtMocHelpers::SignalData<void(int, int, Qt::MouseButton)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 }, { 0x80000000 | 10, 11 },
        }}),
        // Signal 'wheelScrolled'
        QtMocHelpers::SignalData<void(int, int, int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 }, { QMetaType::Int, 9 }, { QMetaType::Int, 16 },
        }}),
        // Slot 'setConnected'
        QtMocHelpers::SlotData<void(bool)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Slot 'updateFrame'
        QtMocHelpers::SlotData<void(const QImage &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 20, 21 },
        }}),
        // Slot 'clear'
        QtMocHelpers::SlotData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<RemoteDesktopWidget, qt_meta_tag_ZN19RemoteDesktopWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject RemoteDesktopWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19RemoteDesktopWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19RemoteDesktopWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19RemoteDesktopWidgetE_t>.metaTypes,
    nullptr
} };

void RemoteDesktopWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RemoteDesktopWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->keyPressed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Qt::KeyboardModifiers>>(_a[2]))); break;
        case 1: _t->keyReleased((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<Qt::KeyboardModifiers>>(_a[2]))); break;
        case 2: _t->mousePressed((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<Qt::MouseButton>>(_a[3]))); break;
        case 3: _t->mouseReleased((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<Qt::MouseButton>>(_a[3]))); break;
        case 4: _t->mouseMoved((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->mouseDoubleClicked((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<Qt::MouseButton>>(_a[3]))); break;
        case 6: _t->wheelScrolled((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3]))); break;
        case 7: _t->setConnected((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->updateFrame((*reinterpret_cast<std::add_pointer_t<QImage>>(_a[1]))); break;
        case 9: _t->clear(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , Qt::KeyboardModifiers )>(_a, &RemoteDesktopWidget::keyPressed, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , Qt::KeyboardModifiers )>(_a, &RemoteDesktopWidget::keyReleased, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , int , Qt::MouseButton )>(_a, &RemoteDesktopWidget::mousePressed, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , int , Qt::MouseButton )>(_a, &RemoteDesktopWidget::mouseReleased, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , int )>(_a, &RemoteDesktopWidget::mouseMoved, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , int , Qt::MouseButton )>(_a, &RemoteDesktopWidget::mouseDoubleClicked, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (RemoteDesktopWidget::*)(int , int , int )>(_a, &RemoteDesktopWidget::wheelScrolled, 6))
            return;
    }
}

const QMetaObject *RemoteDesktopWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RemoteDesktopWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19RemoteDesktopWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RemoteDesktopWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void RemoteDesktopWidget::keyPressed(int _t1, Qt::KeyboardModifiers _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void RemoteDesktopWidget::keyReleased(int _t1, Qt::KeyboardModifiers _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2);
}

// SIGNAL 2
void RemoteDesktopWidget::mousePressed(int _t1, int _t2, Qt::MouseButton _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3);
}

// SIGNAL 3
void RemoteDesktopWidget::mouseReleased(int _t1, int _t2, Qt::MouseButton _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3);
}

// SIGNAL 4
void RemoteDesktopWidget::mouseMoved(int _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void RemoteDesktopWidget::mouseDoubleClicked(int _t1, int _t2, Qt::MouseButton _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3);
}

// SIGNAL 6
void RemoteDesktopWidget::wheelScrolled(int _t1, int _t2, int _t3)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2, _t3);
}
QT_WARNING_POP
