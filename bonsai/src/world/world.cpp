#include "world.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "assets/asset_cache.hpp"
#include "components/camera_component.hpp"
#include "components/render_component.hpp"

/// @brief Parse an entity transform description from the JSON scene format.
/// @param transform_desc Entity transform description.
/// @return A new entity transform representing the description.
static Transform parse_entity_transform(nlohmann::json const& transform_desc)
{
    Transform transform{};
    if (transform_desc.contains("position"))
    {
        std::vector<float> position = transform_desc["position"];
        BONSAI_ASSERT(position.size() == 3 && "Bonsai scene position must contain 3 values!");
        position.resize(3);
        transform.position = { position[0], position[1], position[2] };
    }
    if (transform_desc.contains("rotation"))
    {
        std::vector<float> rotation = transform_desc["rotation"];
        BONSAI_ASSERT(rotation.size() == 4 && "Bonsai scene rotation must contain 4 values!");
        rotation.resize(4);
        transform.rotation = { rotation[0], rotation[1], rotation[2], rotation[4] };
    }
    if (transform_desc.contains("scale"))
    {
        std::vector<float> scale = transform_desc["scale"];
        BONSAI_ASSERT(scale.size() == 3 && "Bonsai scene scale must contain 3 values!");
        scale.resize(3);
        transform.scale = { scale[0], scale[1], scale[2] };
    }

    return transform;
}

/// @brief Parse an entity component description from the JSON scene format.
/// @param entity Entity to update with the component.
/// @param component_desc Component description to use for parsing.
/// @param parent_directory Parent directory of scene file, used for resolving relative asset paths.
static void parse_entity_component(Entity::Ref& entity, nlohmann::json const& component_desc, std::filesystem::path const& parent_directory)
{
    std::string const component_type = component_desc.value("type", "unknown");
    nlohmann::json const component_data = component_desc.value("data", nlohmann::json());
    if (component_type == "render_component")
    {
        std::filesystem::path model_path = component_data.value("model", std::filesystem::path());
        if (!model_path.is_absolute())
        {
            model_path = parent_directory / model_path;
        }
        model_path = model_path.lexically_normal();
        entity->add_component<RenderComponent>(AssetCache::load<Model>(model_path));
    }
    else if (component_type == "camera_component")
    {
        if (component_data.contains("perspective"))
        {
            nlohmann::json const& perspective_desc = component_data["perspective"];
            float const camera_fov = perspective_desc.value("fov", 60.0F);
            float const camera_z_near = perspective_desc.value("z_near", 0.01F);
            float const camera_z_far = perspective_desc.value("z_far", 100.0F);
            entity->add_component<CameraComponent>(Camera(camera_fov, camera_z_near, camera_z_far));
        }
        else
        {
            BONSAI_LOG_WARNING("Encountered unknown camera type: {}", component_data.dump());
        }
    }
    else
    {
        BONSAI_LOG_WARNING("Encountered unknown component type: {}", component_type);
    }
}

World World::from_file(std::filesystem::path const& path)
{
    std::ifstream file(path);
    if (!file)
    {
        BONSAI_LOG_ERROR("Failed to load world file from path [{}]", path.string());
        return {};
    }

    std::filesystem::path const parent_directory = path.parent_path();
    nlohmann::json const world_file = nlohmann::json::parse(file, nullptr, false, true);
    std::string const world_name = world_file.value("name", "Nameless World");
    std::vector<nlohmann::json> const entity_descriptions = world_file.value("entities", std::vector<nlohmann::json>());

    World world{};
    world.set_name(world_name);
    for (auto const& entity_desc : entity_descriptions)
    {
        // Get JSON data
        std::string const entity_name = entity_desc.value("name", "Entity");
        nlohmann::json const transform_desc = entity_desc.value("transform", nlohmann::json());
        std::vector<nlohmann::json> const component_descriptions = entity_desc.value("components", std::vector<nlohmann::json>());
        std::vector<uint32_t> const children = entity_desc.value("children", std::vector<uint32_t>());

        // Create entity
        Entity::Ref entity = Entity::create<Entity>();
        entity->set_name(entity_name);
        entity->set_transform(parse_entity_transform(transform_desc));
        for (auto const& component_desc : component_descriptions)
        {
            parse_entity_component(entity, component_desc, parent_directory);
        }
        world.get_root()->add_child(entity); // TODO(nemjit001): First build out entity list, then separately build out scene tree from "children" descriptors
    }

    return world;
}

void World::update(double delta)
{
    std::vector<Entity::Ref> stack;
    stack.reserve(256); // Reserve space for processing at least 256 nodes to avoid unnecessary allocations
    stack.push_back(m_root);
    while (!stack.empty())
    {
        // Fetch next node from the stack
        Entity::Ref const current = stack.back();
        stack.pop_back();

        // Get entity data before update, copies are used to avoid iterator invalidation during update in entities.
        std::vector<Entity::ComponentRef> components = current->get_components();
        std::vector<Entity::Ref> children = current->get_children();

        // Update all entity components
        for (auto const& component : components)
        {
            component->update(delta);
        }

        // Push entity children onto the stack for further processing
        for (auto const& child : children)
        {
            stack.push_back(child);
        }
    }
}
