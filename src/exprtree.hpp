#ifndef MIXER_LANG_EXPRTREE_HPP_INCLUDED
#define MIXER_LANG_EXPRTREE_HPP_INCLUDED

/*
 Copyright (c) 2017 Andy Little 

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#include <cstdint>
#include "input_output.hpp"

namespace apm_mix{

   template <typename T>
   struct constant final : expr<T>{
      constant(T value_in):m_value{value_in}{}
      T eval()const {return m_value;}
      bool is_constant() const { return true;}
      expr<T>* fold() { return this;}
      expr<T>* clone() const { return new constant{this->m_value};}
   private:
      T m_value;
   };

   template <typename Result , typename Arg>
   struct unary_op final : expr<Result>{
      typedef Result (*unary_fun)( Arg);
      unary_op(unary_fun fun_in,expr<Arg> * arg_in)
      :m_fun{fun_in}, m_arg{(expr<Arg>*)arg_in->fold()}{}
      ~unary_op()
      { 
         if (m_arg != nullptr){delete m_arg;}
      }

      bool is_constant() const { return m_arg->is_constant();}

      expr<Result>* fold()
      {
         if ( m_arg->is_constant() ){
             auto* result = new constant<Result>(this->eval());
             delete this;
             return result;
         }else{
            return this;
         }
      }

      Result eval() const 
      {
         return m_fun(m_arg->eval());
      }

      expr<Result>* clone() const  
      {
         return new unary_op{this->m_fun,(expr<Arg>*)this->m_arg->clone()}; 
      }

   private:
      unary_fun  m_fun;
      expr<Arg> * m_arg;
   };

   template <typename Result , typename Arg>
   struct binary_op final : expr<Result>{
      typedef Result (*binary_fun)( Arg ,Arg);
      binary_op(binary_fun fun_in,expr<Arg> * left_in, expr<Arg> * right_in)
      :m_fun{fun_in}, m_left{(expr<Arg>*)left_in->fold()},m_right{(expr<Arg>*)right_in->fold()}{}
      ~binary_op(){delete m_left; delete m_right;}
      Result eval() const 
      {
         return m_fun(m_left->eval(), m_right->eval());
      }

      bool is_constant() const { return m_left->is_constant() && m_right->is_constant();}
      expr<Result>* fold()
      {
         if (this->is_constant()){
            auto * result = new constant<Result>(this->eval());
            delete this;
            return result;
         }else{
            return this;
         }
      }

      expr<Result>* clone() const  
      {
         return new binary_op{this->m_fun,(expr<Arg>*)this->m_left->clone(),(expr<Arg>*)this->m_right->clone()};
      }

   private:
      binary_fun  m_fun;
      expr<Arg> * m_left;
      expr<Arg> * m_right;
   };

   template <typename T>
   struct if_op final : expr<T>{
         if_op ( expr<bool> * cond_in, expr<T>* true_expr_in, expr<T>* false_expr_in)
         : m_condition{(expr<bool> *)cond_in->fold()}, m_true_expr{(expr<T>*)true_expr_in->fold()},m_false_expr{(expr<T>*)false_expr_in->fold()}{}
         ~if_op(){delete m_condition; if(m_true_expr){delete m_true_expr;} if (m_false_expr){delete m_false_expr;} }
        T eval() const
        {
            if ( m_condition->eval()){
               return m_true_expr->eval();
            }else{
               return m_false_expr->eval();
            }
        }

        bool is_constant() const 
        { 
            if ( m_condition->is_constant()){
               if ( m_condition->eval() ){
                  return  m_true_expr->is_constant();
               }else{
                  return  m_false_expr->is_constant();
               }
            }else{
               return false;
            }
         }

         expr<T> * fold()
         {
             if ( m_condition->is_constant()){
               
               expr<T>* & active_expr = (m_condition->eval())? m_true_expr: m_false_expr;
               if ( active_expr->is_constant()){
                  auto * result = new constant<T>(active_expr->eval());
                  delete this;
                  return result;
               }else{
                  auto* result = active_expr;
                  active_expr = nullptr;
                  delete this;
                  return result;
               }
            }else{
               return this;
            }
         }
         expr<T>* clone() const  
         {
            return new if_op{(expr<bool> *)this->m_condition->clone(),(expr<T>*)this->m_true_expr->clone(),(expr<T>*)this->m_false_expr->clone()};
         }
      private:
         expr<bool> * m_condition;
         expr<T>* m_true_expr;
         expr<T>* m_false_expr;
   };

}

#endif // MIXER_LANG_EXPRTREE_HPP_INCLUDED
