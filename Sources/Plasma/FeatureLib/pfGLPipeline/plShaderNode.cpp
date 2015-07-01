/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "plShaderNode.h"
#include <string_theory/format>

using namespace std;

bool plArgumentNodeSorterFunc(std::shared_ptr<plArgumentNode> a, std::shared_ptr<plArgumentNode> b)
{
    return a->pos < b->pos;
}

ST::string plShaderContext::RenderNode(std::shared_ptr<plShaderNode> node, std::shared_ptr<plShaderFunction> fn)
{
    switch (node->klass)
    {
        case kConstant:
        {
            std::shared_ptr<plConstantNode> c = static_pointer_cast<plConstantNode>(node);

            return c->value;
        }
        break;

        case kAttribute:
        {
            std::shared_ptr<plAttributeNode> attr = static_pointer_cast<plAttributeNode>(node);

            this->attributes.insert(attr);

            return attr->name;
        }
        break;

        case kUniform:
        {
            std::shared_ptr<plUniformNode> uni = static_pointer_cast<plUniformNode>(node);

            this->uniforms.insert(uni);

            return uni->name;
        }
        break;

        case kVarying:
        {
            std::shared_ptr<plVaryingNode> var = static_pointer_cast<plVaryingNode>(node);

            this->varyings.insert(var);

            return var->name;
        }
        break;

        case kTempVar:
        {
            std::shared_ptr<plTempVariableNode> tmp = static_pointer_cast<plTempVariableNode>(node);

            fn->temps.insert(tmp);

            return tmp->name;
        }
        break;

        case kArgument:
        {
            std::shared_ptr<plArgumentNode> arg = static_pointer_cast<plArgumentNode>(node);

            fn->args.insert(arg);

            return arg->name;
        }
        break;

        case kOutput:
        {
            std::shared_ptr<plOutputNode> out = static_pointer_cast<plOutputNode>(node);

            return out->name;
        }
        break;

        case kOperator:
        {
            std::shared_ptr<plOperatorNode> op = static_pointer_cast<plOperatorNode>(node);

            if (op->op == ".") {
              return ST::format("{}{}{}", this->RenderNode(op->lhs, fn), op->op, this->RenderNode(op->rhs, fn));
            } else if (op->parens) {
              return ST::format("({} {} {})", this->RenderNode(op->lhs, fn), op->op, this->RenderNode(op->rhs, fn));
            } else {
              return ST::format("{} {} {}", this->RenderNode(op->lhs, fn), op->op, this->RenderNode(op->rhs, fn));
            }
        }
        break;

        case kAssignment:
        {
            std::shared_ptr<plAssignmentNode> asn = static_pointer_cast<plAssignmentNode>(node);

            return ST::format("{} = {}", this->RenderNode(asn->lhs, fn), this->RenderNode(asn->rhs, fn));
        }
        break;

        case kReturn:
        {
            std::shared_ptr<plReturnNode> ret = static_pointer_cast<plReturnNode>(node);

            return ST::format("return {}", this->RenderNode(ret->var, fn));
        }
        break;

        case kFnCall:
        {
            std::shared_ptr<plCallNode> call = static_pointer_cast<plCallNode>(node);

            std::vector<ST::string> params;

            for (std::shared_ptr<plShaderNode> arg : call->args) {
                params.push_back(this->RenderNode(arg, fn));
            }

            ST::string_stream out;
            out << call->function << "(";

            for (size_t i = 0; i < call->args.size(); i++) {
                if (i > 0) {
                    out << ", ";
                }
                out << params[i];
            }
            out << ")";

            return out.to_string();
        }
        break;

        case kConditional:
        {
            std::shared_ptr<plConditionNode> cond = static_pointer_cast<plConditionNode>(node);

            return ST::format("if ({}) {{ {}; }", this->RenderNode(cond->condition, fn), this->RenderNode(cond->body, fn));
        }
        break;

        default:
        return "";
    }
}

ST::string plShaderContext::Render()
{
    std::vector<ST::string> lines;

    for (std::shared_ptr<plShaderFunction> fn : this->funcs) {
        for (std::shared_ptr<plShaderNode> node : fn->nodes) {
            fn->output.push_back(this->RenderNode(node, fn));
        }
    }


    ST::string_stream out;

    out << ST::format("#version {}\n", this->version);

    if (this->type == kFragment)
        out << "precision mediump float;\n";

    for (std::shared_ptr<plAttributeNode> node : this->attributes)
        out << ST::format("attribute {} {};\n", node->type, node->name);

    for (std::shared_ptr<plUniformNode> node : this->uniforms)
        out << ST::format("uniform {} {};\n", node->type, node->name);

    for (std::shared_ptr<plVaryingNode> node : this->varyings)
        out << ST::format("varying {} {};\n", node->type, node->name);


    for (std::shared_ptr<plShaderFunction> fn : this->funcs) {
        out << ST::format("\n{} {}(", fn->type, fn->name);

        size_t i = 0;
        for (std::shared_ptr<plArgumentNode> arg : fn->args) {
            if (i > 0)
                out << ", ";

            out << ST::format("{} {}", arg->type, arg->name);
            i++;
        }

        out << ") {\n";


        for (std::shared_ptr<plTempVariableNode> node : fn->temps)
            out << "\t" << ST::format("{} {};\n", node->type, node->name);


        if (fn->temps.size())
            out << "\n";


        for (ST::string ln : fn->output)
            out << "\t" << ln << ";\n";

        out << "}\n";
    }

    return out.to_string();
}
