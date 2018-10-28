#include <spark/SPARK.h>
#include <sparkparticles/SparkParticleEmitter.h>
#include <sparkparticles/SparkQuadRenderer.h>
#include <sparkparticles/SparkParticleEmitter.h>

#include "SpkSystem.h"
#include "../../GPDevice.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>


//------------------------------------------------------------------------------------------------------------------------------
// group list
//------------------------------------------------------------------------------------------------------------------------------

NodeSparkGroupList::NodeSparkGroupList()
{
    IN_PORT(ENC_GROUP, "group");
    IN_PORT(ENC_GROUP, "group");
    IN_PORT(ENC_GROUP, "group");
    IN_PORT(ENC_GROUP, "group");
    IN_PORT(ENC_GROUP, "group");
    OUT_PORT(ENC_GROUP, "groups");
}

void NodeSparkGroupList::process()
{
    _groups.clear();
    for(size_t i=0; i<_inputs.size(); i++)
    {
        std::shared_ptr<NodeDataSparkGroupList> in = getInput<NodeDataSparkGroupList>(i);
        if(in.get())
        {
            for(size_t j=0; j<in->_result.size(); j++)
                _groups.push_back(in->_result[j]);
        }
    }

    dataUpdated(0);
}


//--------------------------------------------------------------------------------------------
// group
//--------------------------------------------------------------------------------------------

NodeSparkGroup::NodeSparkGroup()
{
    IN_PORT(ENC_RENDERER, "renderer");
    IN_PORT(ENC_EMITTER, "emitters");
    IN_PORT(ENC_COLORINTERPOLATOR, "colors");
    IN_PORT(ENC_PARAMINTERPOLATOR, "params");
    IN_PORT(ENC_MODIFIER, "modifiers");
    OUT_PORT(ENC_GROUP, "group");

    //createBaseObjectParams("Group");
    PARAM_INT("Capacity", 1, 500000, 100);
    PARAM_FXY("Lifetime", 0.000001, eF32_MAX, 1.0f, 1.0f);
    PARAM_BOOL("Immortal", false);
    PARAM_FLOAT("GraphicalRadius", 0, eF32_MAX, 1.0f);
    PARAM_FLOAT("PhysicalRadius", 0, eF32_MAX, 0.0f);
    PARAM_BOOL("Sorted", false);
}

void NodeSparkGroup::process()
{
    const unsigned int INPUT_RENDERER_INDEX = 0;
    const unsigned int INPUT_EMITTERS_INDEX = 1;
    const unsigned int INPUT_COLORINTERPOLATOR_INDEX = 2;
    const unsigned int INPUT_PARAMSINTERPOLATOR_INDEX = 3;
    const unsigned int INPUT_MODIFIERS_INDEX = 4;

    // get parameters
    eInt capacity = getParameter("Capacity")->getValueAsInt();
    eFXY lifetime = getParameter("Lifetime")->getValueAsFXY();
    bool immortal = getParameter("Immortal")->getValueAsBool();
    eF32 graphicalRadius = getParameter("GraphicalRadius")->getValueAsFloat();
    eF32 physicalRadius = getParameter("PhysicalRadius")->getValueAsFloat();
    bool sorted = getParameter("Sorted")->getValueAsBool();

    // create new group
    SPK::Ref<SPK::Group> group = SPK::Group::create(capacity);
    //setBaseObjectParams(group);
    group->setLifeTime(lifetime.x, lifetime.y);
    group->setImmortal(immortal);
    group->setGraphicalRadius(graphicalRadius);
    group->setPhysicalRadius(physicalRadius);
    group->enableSorting(sorted);

    // set renderer
    std::shared_ptr<NodeDataSparkRenderer> inRenderer = getInput<NodeDataSparkRenderer>(INPUT_RENDERER_INDEX);
    if(inRenderer && inRenderer->_result.get())
    {
        group->setRenderer(inRenderer->_result);
    }
    else
    {
        setValidationState(NodeValidationState::Error, "Missing renderer input");
        return;
    }

    // add emitters
    std::shared_ptr<NodeDataSparkEmitterList> inEmitters = getInput<NodeDataSparkEmitterList>(INPUT_EMITTERS_INDEX);
    if(inEmitters)
    {
        for(size_t i=0; i<inEmitters->_result.size(); i++)
        {
            if(inEmitters->_result[i].get())
                group->addEmitter(inEmitters->_result[i]);
        }
    }

    // add modifiers
    std::shared_ptr<NodeDataSparkModifierList> inModifiers = getInput<NodeDataSparkModifierList>(INPUT_MODIFIERS_INDEX);
    if(inModifiers)
    {
        for(size_t i=0; i<inModifiers->_result.size(); i++)
        {
            if(inModifiers->_result[i].get())
                group->addModifier(inModifiers->_result[i]);
        }
    }

    // set color interpolator
    std::shared_ptr<NodeDataSparkColorInterpolator> inColors = getInput<NodeDataSparkColorInterpolator>(INPUT_COLORINTERPOLATOR_INDEX);
    if(inColors)
    {
        group->setColorInterpolator(inColors->_result);
    }

    // set param interpolators
    std::shared_ptr<NodeDataSparkParamInterpolatorList> inParams = getInput<NodeDataSparkParamInterpolatorList>(INPUT_PARAMSINTERPOLATOR_INDEX);
    if(inParams && inParams.get())
    {
        for(size_t i=0; i<inParams->_result.size(); i++)
        {
            if(inParams->_result[i].interpolatorFloat)
                group->setParamInterpolator(inParams->_result[i].param, inParams->_result[i].interpolatorFloat);
        }
    }

    // store result
    if(_groups.size() > 1)
        _groups.back().reset();
    _groups.clear();
    _groups.push_back(group);

    setValidationState(NodeValidationState::Valid);

    dataUpdated(0);
}

//--------------------------------------------------------------------------------------------
// system
//--------------------------------------------------------------------------------------------

NodeSparkSystem::NodeSparkSystem()
{
    IN_PORT(ENC_GROUP, "groups");

    PARAM_STRING("Name", "System");
    PARAM_BOOL("Initialized", eTRUE);
}

void NodeSparkSystem::process()
{
    QString name = getParameter("Name")->getValueAsString();
    bool initialized = getParameter("Initialized")->getValueAsBool();

    /*if(!_system.get())
        _system = SPK::System::create(true);
    else
        _system->removeAllGroups();*/

    _system = SPK::System::create(initialized);
    _system->setName(name.toStdString());

    // add groups
    std::shared_ptr<NodeDataSparkGroupList> in0 = getInput<NodeDataSparkGroupList>(0);
    if(in0)
    {
        for(size_t i=0; i<in0->_result.size(); i++)
        {
            if(in0->_result[i].get())
                _system->addGroup(in0->_result[i]);
        }
    }

    GplayDevice::get().setCurentParticleSystem(_system);
}


//--------------------------------------------------------------------------------------------
// quad renderer node
//--------------------------------------------------------------------------------------------

NodeSparkQuadRenderer::NodeSparkQuadRenderer()
{
    OUT_PORT(ENC_RENDERER, "renderer");

    createBaseObjectParams("Renderer");
    PARAM_FILE("Material", "res/data/materials/particle.material");
    PARAM_FILE("Texture", "res/data/textures/flare.png");
    PARAM_IXY("AtlasDimension", 1, 1000, 1, 1);
    PARAM_FXY("Scale", 0.0f, eF32_MAX, 1.0f, 1.0f);
    PARAM_ENUM("Orientation", "CAMERA_PLANE_ALIGNED"
                              "|CAMERA_POINT_ALIGNED"
                              "|DIRECTION_ALIGNED"
                              "|AROUND_AXIS"
                              "|TOWARDS_POINT"
                              "|FIXED_ORIENTATION", 0);
}

Material* createDefaultMaterial()
{
    // Create a material for particles
    Material* material = Material::create("res/core/shaders/particle.vert", "res/core/shaders/particle.frag");
    Texture::Sampler* sampler = material->getParameter("u_diffuseTexture")->setValue("res/data/textures/flare.png", true);
    sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
    material->getStateBlock()->setCullFace(true);
    material->getStateBlock()->setDepthTest(true);
    material->getStateBlock()->setDepthWrite(false);
    material->getStateBlock()->setBlend(true);
    material->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
    material->getStateBlock()->setBlendDst(RenderState::BLEND_ONE);

    return material;
}

void NodeSparkQuadRenderer::process()
{
    // get parameters
    std::string materialFile(getParameter("Material")->getValueAsString().toStdString());
    std::string textureFile(getParameter("Texture")->getValueAsString().toStdString());
    eIXY atlasDimensions = (getParameter("AtlasDimension")->getValueAsIXY());
    eFXY scale = getParameter("Scale")->getValueAsFXY();
    SPK::OrientationPreset orientation = SPK::OrientationPreset(getParameter("Orientation")->getValueAsEnum());

    bool isMaterialFileExists = gplay::FileSystem::fileExists(materialFile.c_str());
    bool isTextureFileExists = gplay::FileSystem::fileExists(textureFile.c_str());

    Material* material = createDefaultMaterial();
    material->getParameter("u_diffuseTexture")->setValue(textureFile.c_str(), true);

    // Renderer
    SPK::Ref<SPK::GP3D::SparkQuadRenderer> renderer = SPK::GP3D::SparkQuadRenderer::create();
    renderer->setTexturingMode(SPK::TEXTURE_MODE_2D);
    renderer->setBlendMode(SPK::BLEND_MODE_ADD);
    renderer->enableRenderingOption(SPK::RENDERING_OPTION_DEPTH_WRITE,false);
    renderer->setScale(scale.x, scale.y);
    renderer->setMaterial(material->clone());
    renderer->setAtlasDimensions(atlasDimensions.x, atlasDimensions.y);
    renderer->setOrientation(orientation);

    _renderer.reset();
    _renderer = renderer;

    dataUpdated(0);
}



//------------------------------------------------------------------------------------------------------------------------------
// test node
//------------------------------------------------------------------------------------------------------------------------------

#include <QMessageBox>
#include "../common/CustomWidgets.h"

NodeSparkTest::NodeSparkTest()
{
    OUT_PORT(ENC_RENDERER, "renderer");

    PARAM_TEXT("text", "Bonjour");
    PARAM_STRING("string", "Hello");
    PARAM_BOOL("bool", false);
    PARAM_INT("int", -5, 5, 2);
    PARAM_FLOAT("float", -5.0f, 5.0f, 3.14f);
    PARAM_FXY("fxy", -5.0f, 5.0f, 1.0f, 2.0f);
    PARAM_FXYZ("fxyz", -5.0f, 5.0f, 1.0f, 2.0f, 3.0f);
    PARAM_FXYZW("fxyzw", -5.0f, 5.0f, 1.0f, 2.0f, 3.0f, 4.0f);
    PARAM_IXY("ixy", -5, 5, 1, 2);
    PARAM_IXYZ("ixyz", -5, 5, 1, 2, 3);
    PARAM_IXYZW("ixyzw", -5, 5, 1, 2, 3, 4);
    PARAM_ENUM("enum", "enum1|enum2|enum3|num4|enum5", 1);
    PARAM_FILE("file", "res/data/textures");
    PARAM_FLAGS("flags", "flag1|flag2|flag3|flag4|flag5", 2)
    PARAM_RGBA("rgba", 100, 50, 50, 255);
    PARAM_BUTTON("button", "Simple Button");



    // force parameters widget creation  immediately to get acess to pushbutton ptr
    createParamWidgets();

    // connect push button to a slot
    eButton* button = (eButton*)getParameter("button")->userData(0);
    Q_ASSERT(button);
    connect(button, &QPushButton::clicked, this, &NodeSparkTest::onTestButtonClick);
}

void NodeSparkTest::process()
{

}

void NodeSparkTest::onTestButtonClick()
{
    QMessageBox msgBox;
    msgBox.setText("The button has been clicked.");
    msgBox.exec();
}
