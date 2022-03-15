#include "bump_assets.hpp"

#include "bump_die.hpp"
#include "bump_ends_with.hpp"
#include "bump_app.hpp"
#include "bump_load_gl_texture.hpp"
#include "bump_log.hpp"

#include <array>
#include <fstream>

namespace bump
{

	namespace
	{

		std::string read_file_to_string(std::string const& filename)
		{
			auto file = std::ifstream(filename, std::ios::binary); // todo: widen filename for windows!
			return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}

		GLenum get_shader_type(std::string const& filename)
		{
			if (ends_with(filename, ".vert"))
				return GL_VERTEX_SHADER;

			if (ends_with(filename, ".tesc"))
				return GL_TESS_CONTROL_SHADER;

			if (ends_with(filename, ".tese"))
				return GL_TESS_EVALUATION_SHADER;

			if (ends_with(filename, ".geom"))
				return GL_GEOMETRY_SHADER;

			if (ends_with(filename, ".frag"))
				return GL_FRAGMENT_SHADER;

			log_error("Unrecognised shader file extension: " + filename);
			die();
		}

	} // unnamed
	
	assets load_assets(app& app, asset_metadata const& m)
	{
		auto out = assets();

		// load fonts:
		{
			for (auto const& metadata : m.m_fonts)
			{
				auto ft_font = font::ft_font(app.m_ft_context.get_handle(), "data/fonts/" + metadata.m_filename);
				ft_font.set_pixel_size(metadata.m_size_pixels_per_em);

				auto hb_font = font::hb_font(ft_font.get_handle());

				if (!out.m_fonts.insert({ metadata.m_name, font::font_asset{ std::move(ft_font), std::move(hb_font) } }).second)
				{
					log_error("load_assets(): duplicate font id: " + metadata.m_name);
					die();
				}
			}
		}

		// load sounds:
		{
			for (auto const& metadata : m.m_sounds)
			{
				auto chunk = sdl::mixer_chunk("data/sounds/" + metadata.m_filename);

				if (!out.m_sounds.insert({ metadata.m_name, std::move(chunk) }).second)
				{
					log_error("load_assets(): duplicate sound id: " + metadata.m_name);
					die(); 
				}
			}
		}
		
		// load music:
		{
			for (auto const& metadata : m.m_music)
			{
				auto chunk = sdl::mixer_music("data/music/" + metadata.m_filename);

				if (!out.m_music.insert({ metadata.m_name, std::move(chunk) }).second)
				{
					log_error("load_assets(): duplicate music id: " + metadata.m_name);
					die(); 
				}
			}
		}

		// load shaders:
		{
			for (auto const& metadata : m.m_shaders)
			{
				auto objects = std::vector<gl::shader_object>();

				for (auto const& file : metadata.m_filenames)
				{
					auto source = read_file_to_string("data/shaders/" + file);
					auto type = get_shader_type(file);

					auto object = gl::shader_object(type);
					object.set_source(source);

					if (!object.compile())
					{
						log_error("Failed to compile shader object: " + file + " for shader asset: " + metadata.m_name);
						log_error(object.get_log());
					}

					objects.push_back(std::move(object));
				}

				die_if(!std::all_of(objects.begin(), objects.end(), [] (gl::shader_object const& o) { return o.is_compiled(); }));

				auto shader = gl::shader_program();

				for (auto const& object : objects)
					shader.attach(object);
				
				if (!shader.link())
				{
					log_error("Failed to link shader program: " + metadata.m_name);
					log_error(shader.get_log());
					die();
				}

				for (auto const& object : objects)
					shader.detach(object);
				
				if (!out.m_shaders.insert({ metadata.m_name, std::move(shader) }).second)
				{
					log_error("load_assets(): duplicate shader id: " + metadata.m_name);
					die();
				}
			}
		}

		// load models:
		{
			for (auto const& metadata : m.m_models)
			{
				auto model = load_mbp_model_json("data/models/" + metadata.m_filename);

				if (!out.m_models.insert({ metadata.m_name, std::move(model) }).second)
				{
					log_error("load_assets(): duplicate model id: " + metadata.m_name);
					die();
				}
			}
		}

		// load 2d array textures:
		{
			for (auto const& metadata : m.m_texture_2d_arrays)
			{
				auto file = "data/textures/" + metadata.m_filename;

				auto texture = load_gl_texture_2d_array_from_file(file, metadata.m_num_layers, metadata.m_parameters);

				if (!out.m_texture_2d_arrays.insert({ metadata.m_name, std::move(texture) }).second)
				{
					log_error("load_assets(): duplicate texture_2d_array id: " + metadata.m_name);
					die();
				}
			}
		}

		// load cubemap textures:
		{
			for (auto const& metadata : m.m_texture_cubemaps)
			{
				auto files = std::array<std::string, 6>();

				for (auto i = std::size_t{ 0 }; i != files.size(); ++i)
					files[i] = "data/textures/" + metadata.m_filenames[i];

				auto cubemap = load_gl_cubemap_texture_from_files(files, metadata.m_parameters);

				if (!out.m_texture_cubemaps.insert({ metadata.m_name, std::move(cubemap) }).second)
				{
					log_error("load_assets(): duplicate texture_cubemap id: " + metadata.m_name);
					die();
				}
			}
		}

		return out;
	}
	
} // bump
