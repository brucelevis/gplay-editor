#ifndef ZONENODE_H
#define ZONENODE_H

#include "BaseNode.h"

//------------------------------------------------------------------------------------------------------------------------------
// base spark zone class
//------------------------------------------------------------------------------------------------------------------------------
class NodeSparkZoneBase : public NodeSparkBaseNode
{
protected:
    void createBaseZoneParams(eString name, bool shared = false);
    void setBaseZoneParams(SPK::Ref<SPK::Zone> zone);
    void setResult(SPK::Ref<SPK::Zone> zone);

    SPK::Ref<SPK::Zone> _zone;
};


//------------------------------------------------------------------------------------------------------------------------------
// point zone node
//------------------------------------------------------------------------------------------------------------------------------
class NodeSparkZonePoint : public NodeSparkZoneBase
{
private:
    const QString Name() const override { return QString("PointZone"); }
    std::shared_ptr<NodeData> outData(PortIndex) override { return std::make_shared<NodeDataSparkZone>(_zone); }
    void process() override;
public:
    NodeSparkZonePoint();
};

//------------------------------------------------------------------------------------------------------------------------------
// plane zone node
//------------------------------------------------------------------------------------------------------------------------------
class NodeSparkZonePlane : public NodeSparkZoneBase
{
private:
    const QString Name() const override { return QString("PlaneZone"); }
    std::shared_ptr<NodeData> outData(PortIndex) override { return std::make_shared<NodeDataSparkZone>(_zone); }
    void process() override;
public:
    NodeSparkZonePlane();
};


//------------------------------------------------------------------------------------------------------------------------------
// sphere zone node
//------------------------------------------------------------------------------------------------------------------------------
class NodeSparkZoneSphere : public NodeSparkZoneBase
{
private:
    const QString Name() const override { return QString("SphereZone"); }
    std::shared_ptr<NodeData> outData(PortIndex) override { return std::make_shared<NodeDataSparkZone>(_zone); }
    void process() override;
public:
    NodeSparkZoneSphere();
};


//------------------------------------------------------------------------------------------------------------------------------
// box zone node
//------------------------------------------------------------------------------------------------------------------------------
class NodeSparkZoneBox : public NodeSparkZoneBase
{
private:
    const QString Name() const override { return QString("BoxZone"); }
    std::shared_ptr<NodeData> outData(PortIndex) override { return std::make_shared<NodeDataSparkZone>(_zone); }
    void process() override;
public:
    NodeSparkZoneBox();
};

#endif // ZONENODE_H
